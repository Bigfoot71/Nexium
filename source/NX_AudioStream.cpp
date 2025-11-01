/* NX_AudioStream.cpp -- API definition for Nexium's audio stream module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_AudioStream.h>
#include <NX/NX_Filesystem.h>

#include "Detail/Util/DynamicArray.hpp"
#include "./Detail/Util/Memory.hpp"
#include "./INX_AudioFormat.hpp"
#include "./NX_AudioStream.hpp"
#include "./INX_PoolAssets.hpp"

#include <SDL3/SDL_assert.h>

#include <condition_variable>
#include <algorithm>
#include <thread>
#include <atomic>
#include <mutex>
#include <array>

// ============================================================================
// DECODER HELPERS
// ============================================================================

static bool INX_InitDecoder(NX_AudioStream::Decoder* decoder, int* channels, const uint8_t* data, size_t size, INX_AudioFormat format)
{
    SDL_assert(decoder != nullptr && channels != nullptr);

    switch (format) {
    case INX_AudioFormat::WAV:
        decoder->wav = NX_Malloc<drwav>();
        if (!drwav_init_memory(decoder->wav, data, size, nullptr)) {
            return false;
        }
        *channels = decoder->wav->channels;
        break;
    case INX_AudioFormat::FLAC:
        decoder->flac = drflac_open_memory(data, size, nullptr);
        if (!decoder->flac) return false;
        *channels = decoder->flac->channels;
        break;
    case INX_AudioFormat::MP3:
        decoder->mp3 = NX_Malloc<drmp3>();
        if (!drmp3_init_memory(decoder->mp3, data, size, nullptr)) {
            return false;
        }
        *channels = decoder->mp3->channels;
        break;
    case INX_AudioFormat::OGG:
        decoder->ogg = stb_vorbis_open_memory(data, size, nullptr, nullptr);
        if (!decoder->ogg) return false;
        *channels = stb_vorbis_get_info(decoder->ogg).channels;
        break;
    default:
        return false;
    }

    return true;
}

static void INX_DestroyDecoder(NX_AudioStream::Decoder& decoder, INX_AudioFormat format)
{
    switch (format) {
    case INX_AudioFormat::WAV:
        drwav_uninit(decoder.wav);
        NX_Free(decoder.wav);
        break;
    case INX_AudioFormat::FLAC:
        drflac_close(decoder.flac);
        break;
    case INX_AudioFormat::MP3:
        drmp3_uninit(decoder.mp3);
        NX_Free(decoder.mp3);
        break;
    case INX_AudioFormat::OGG:
        stb_vorbis_close(decoder.ogg);
        break;
    default:
        break;
    }
}

static int INX_GetChannelCount(const NX_AudioStream& stream)
{
    switch (stream.audioFormat) {
    case INX_AudioFormat::WAV:  return stream.decoder.wav->channels;
    case INX_AudioFormat::FLAC: return stream.decoder.flac->channels;
    case INX_AudioFormat::MP3:  return stream.decoder.mp3->channels;
    case INX_AudioFormat::OGG:  return stb_vorbis_get_info(stream.decoder.ogg).channels;
    default: return 0;
    }
}

static int INX_GetSampleRate(const NX_AudioStream& stream)
{
    switch (stream.audioFormat) {
    case INX_AudioFormat::WAV:  return stream.decoder.wav->sampleRate;
    case INX_AudioFormat::FLAC: return stream.decoder.flac->sampleRate;
    case INX_AudioFormat::MP3:  return stream.decoder.mp3->sampleRate;
    case INX_AudioFormat::OGG:  return stb_vorbis_get_info(stream.decoder.ogg).sample_rate;
    default: return 0;
    }
}

static size_t INX_DecodeSamples(const NX_AudioStream& stream, void* buffer, size_t samples)
{
    switch (stream.audioFormat) {
    case INX_AudioFormat::WAV:
        return drwav_read_pcm_frames_s16(stream.decoder.wav, samples, static_cast<drwav_int16*>(buffer));
    case INX_AudioFormat::FLAC:
        return drflac_read_pcm_frames_s16(stream.decoder.flac, samples, static_cast<drflac_int16*>(buffer));
    case INX_AudioFormat::MP3:
        return drmp3_read_pcm_frames_s16(stream.decoder.mp3, samples, static_cast<drmp3_int16*>(buffer));
    case INX_AudioFormat::OGG: {
        int channels = stb_vorbis_get_info(stream.decoder.ogg).channels;
        return stb_vorbis_get_samples_short_interleaved(
            stream.decoder.ogg, channels, 
            static_cast<int16_t*>(buffer), 
            samples * channels
        );
    }
    default:
        return 0;
    }
}

static void INX_SeekToStart(const NX_AudioStream& stream)
{
    switch (stream.audioFormat) {
    case INX_AudioFormat::WAV:  drwav_seek_to_pcm_frame(stream.decoder.wav, 0); break;
    case INX_AudioFormat::FLAC: drflac_seek_to_pcm_frame(stream.decoder.flac, 0); break;
    case INX_AudioFormat::MP3:  drmp3_seek_to_pcm_frame(stream.decoder.mp3, 0); break;
    case INX_AudioFormat::OGG:  stb_vorbis_seek(stream.decoder.ogg, 0); break;
    default: break;
    }
}

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

NX_AudioStream::~NX_AudioStream()
{
    INX_DestroyDecoder(decoder, audioFormat);

    if (source > 0) {
        alDeleteSources(1, &source);
        alDeleteBuffers(BufferCount, buffers.data());
    }
}

// ============================================================================
// STREAM PLAYER
// ============================================================================

struct INX_StreamPlayer {
    static constexpr size_t MaxDecodeBuffers = 32;
    static constexpr size_t DecodeBufferSize = NX_AudioStream::BufferSize;

    std::thread streamThread;
    std::mutex streamMutex;      // For activeStreams
    std::mutex bufferMutex;      // For the buffer pool
    std::condition_variable cv;
    std::atomic<bool> shouldStop{false};

    util::DynamicArray<NX_AudioStream*> activeStreams;
    std::array<util::UniquePtr<uint8_t>, MaxDecodeBuffers> decodeBuffers;
    std::array<bool, MaxDecodeBuffers> bufferAvailable;

    INX_StreamPlayer();
    ~INX_StreamPlayer();

    void addStream(NX_AudioStream* stream);
    void removeStream(NX_AudioStream* stream);
    void prepareStream(NX_AudioStream* stream);

    uint8_t* requestBuffer();
    void releaseBuffer(uint8_t* buffer);

private:
    void threadFunc();
    void updateStreams();
    bool updateStream(NX_AudioStream* stream);
    bool fillBuffer(NX_AudioStream* stream, ALuint buffer);
};

INX_StreamPlayer::INX_StreamPlayer()
{
    bufferAvailable.fill(true);
    for (auto& buffer : decodeBuffers) {
        buffer = util::makeUnique<uint8_t>(DecodeBufferSize);
    }

    streamThread = std::thread([this]() { threadFunc(); });
}

INX_StreamPlayer::~INX_StreamPlayer()
{
    shouldStop = true;
    cv.notify_one();

    if (streamThread.joinable()) {
        streamThread.join();
    }
}

void INX_StreamPlayer::addStream(NX_AudioStream* stream)
{
    std::lock_guard<std::mutex> lock(streamMutex);

    auto it = std::find(activeStreams.begin(), activeStreams.end(), stream);
    if (it == activeStreams.end()) {
        activeStreams.push_back(stream);
        cv.notify_one();
    }
}

void INX_StreamPlayer::removeStream(NX_AudioStream* stream)
{
    std::lock_guard<std::mutex> lock(streamMutex);

    auto it = std::find(activeStreams.begin(), activeStreams.end(), stream);
    if (it != activeStreams.end()) {
        activeStreams.erase(it);
    }
}

void INX_StreamPlayer::prepareStream(NX_AudioStream* stream)
{
    for (size_t i = 0; i < NX_AudioStream::BufferCount; ++i) {
        fillBuffer(stream, stream->buffers[i]);
    }
}

uint8_t* INX_StreamPlayer::requestBuffer()
{
    std::lock_guard<std::mutex> lock(bufferMutex);

    for (size_t i = 0; i < MaxDecodeBuffers; ++i) {
        if (bufferAvailable[i]) {
            bufferAvailable[i] = false;
            return decodeBuffers[i].get();
        }
    }

    return nullptr;
}

void INX_StreamPlayer::releaseBuffer(uint8_t* buffer)
{
    std::lock_guard<std::mutex> lock(bufferMutex);

    for (size_t i = 0; i < MaxDecodeBuffers; ++i) {
        if (decodeBuffers[i].get() == buffer) {
            bufferAvailable[i] = true;
            break;
        }
    }
}

bool INX_StreamPlayer::fillBuffer(NX_AudioStream* stream, ALuint buffer)
{
    uint8_t* decodeBuffer = requestBuffer();
    if (!decodeBuffer) {
        return false;
    }

    int channels = INX_GetChannelCount(*stream);
    int sampleRate = INX_GetSampleRate(*stream);
    size_t samplesToRead = DecodeBufferSize / (channels * sizeof(int16_t));

    size_t samplesRead = INX_DecodeSamples(*stream, decodeBuffer, samplesToRead);

    if (samplesRead == 0 && stream->shouldLoop) {
        INX_SeekToStart(*stream);
        samplesRead = INX_DecodeSamples(*stream, decodeBuffer, samplesToRead);
    }

    bool success = false;
    if (samplesRead > 0) {
        size_t dataSize = samplesRead * channels * sizeof(int16_t);
        alBufferData(buffer, stream->format, decodeBuffer, dataSize, sampleRate);
        alSourceQueueBuffers(stream->source, 1, &buffer);
        success = true;
    }

    releaseBuffer(decodeBuffer);
    return success;
}

bool INX_StreamPlayer::updateStream(NX_AudioStream* stream)
{
    if (stream->isPaused) {
        return false;
    }

    ALint sourceState;
    alGetSourcei(stream->source, AL_SOURCE_STATE, &sourceState);

    ALint processed = 0;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed);

    bool endOfStream = false;

    while (processed > 0 && !shouldStop) {
        ALuint buffer;
        alSourceUnqueueBuffers(stream->source, 1, &buffer);
        if (!fillBuffer(stream, buffer)) endOfStream = true;
        processed--;
    }

    ALint queued = 0;
    alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);

    if (queued == 0 && endOfStream) {
        INX_SeekToStart(*stream);
        stream->isPlaying = false;
        prepareStream(stream);
        return true;
    }
    else if (sourceState != AL_PLAYING && !stream->isPaused && queued > 0) {
        alSourcePlay(stream->source);
    }

    return false;
}

void INX_StreamPlayer::updateStreams()
{
    std::lock_guard<std::mutex> lock(streamMutex);

    for (size_t i = activeStreams.size(); i > 0; --i) {
        size_t idx = i - 1;
        NX_AudioStream* stream = activeStreams[idx];
        if (stream && updateStream(stream)) {
            activeStreams.erase(activeStreams.begin() + idx);
        }
    }
}

void INX_StreamPlayer::threadFunc()
{
    while (!shouldStop)
    {
        {
            std::unique_lock<std::mutex> lock(streamMutex);
            cv.wait(lock, [this]() { 
                return !activeStreams.empty() || shouldStop.load(); 
            });

            if (shouldStop) break;
        }

        updateStreams();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

static INX_StreamPlayer& INX_GetStreamPlayer()
{
    static INX_StreamPlayer player;
    return player;
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_AudioStream* NX_LoadAudioStream(const char* filePath)
{
    if (!filePath) {
        NX_LOG(E, "AUDIO: File path is null");
        return nullptr;
    }

    /* --- Load the file --- */

    size_t fileSize = 0;
    util::UniquePtr<uint8_t> fileData(static_cast<uint8_t*>(NX_LoadFile(filePath, &fileSize)));
    if (!fileData) {
        NX_LOG(E, "AUDIO: Failed to load file: %s", filePath);
        return nullptr;
    }

    /* --- Determine the format --- */

    INX_AudioFormat audioFormat = INX_GetAudioFormat(fileData.get(), fileSize);
    if (audioFormat == INX_AudioFormat::Unknown) {
        NX_LOG(E, "AUDIO: Unknown format: %s", filePath);
        return nullptr;
    }

    /* --- Initialize the decoder --- */

    NX_AudioStream::Decoder decoder{};
    int channels{};

    if (!INX_InitDecoder(&decoder, &channels, fileData.get(), fileSize, audioFormat)) {
        NX_LOG(E, "AUDIO: Failed to init audio stream decoder");
        return nullptr;
    }

    if (channels == 0 || channels > 2) {
        NX_LOG(E, "AUDIO: Unsupported channel count: %i", channels);
        INX_DestroyDecoder(decoder, audioFormat);
        return nullptr;
    }

    /* --- Create OpenAL resources --- */

    ALenum format = (channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    
    std::array<ALuint, NX_AudioStream::BufferCount> buffers{};
    alGenBuffers(NX_AudioStream::BufferCount, buffers.data());
    if (alGetError() != AL_NO_ERROR) {
        NX_LOG(E, "AUDIO: Failed to create buffers");
        INX_DestroyDecoder(decoder, audioFormat);
        return nullptr;
    }

    ALuint source{};
    alGenSources(1, &source);
    if (alGetError() != AL_NO_ERROR) {
        NX_LOG(E, "AUDIO: Failed to create source");
        alDeleteBuffers(NX_AudioStream::BufferCount, buffers.data());
        INX_DestroyDecoder(decoder, audioFormat);
        return nullptr;
    }

    /* --- Create the stream --- */

    NX_AudioStream* stream = INX_Pool.Create<NX_AudioStream>();
    
    stream->buffers = buffers;
    stream->source = source;
    stream->format = format;
    stream->audioData = std::move(fileData);
    stream->audioFormat = audioFormat;
    stream->decoder = decoder;

    INX_GetStreamPlayer().prepareStream(stream);

    return stream;
}

void NX_DestroyAudioStream(NX_AudioStream* stream)
{
    INX_Pool.Destroy(stream);
}

void NX_PlayAudioStream(NX_AudioStream* stream)
{
    if (stream->isPaused && stream->isPlaying) {
        alSourcePlay(stream->source);
        stream->isPaused = false;
        return;
    }

    INX_GetStreamPlayer().addStream(stream);
    alSourcePlay(stream->source);
    
    stream->isPaused = false;
    stream->isPlaying = true;
}

void NX_PauseAudioStream(NX_AudioStream* stream)
{
    if (stream->isPlaying && !stream->isPaused) {
        alSourcePause(stream->source);
        stream->isPaused = true;
    }
}

void NX_StopAudioStream(NX_AudioStream* stream)
{
    if (!stream->isPlaying) return;

    INX_GetStreamPlayer().removeStream(stream);
    alSourceStop(stream->source);

    ALint queued = 0;
    alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);
    if (queued > 0) {
        ALuint buffers[NX_AudioStream::BufferCount];
        alSourceUnqueueBuffers(stream->source, queued, buffers);
    }

    INX_SeekToStart(*stream);
    stream->isPaused = false;
    stream->isPlaying = false;

    INX_GetStreamPlayer().prepareStream(stream);
}

void NX_RewindAudioStream(NX_AudioStream* stream)
{
    bool wasPlaying = (stream->isPlaying && !stream->isPaused);

    if (stream->isPaused) {
        alSourceStop(stream->source);
        ALint queued = 0;
        alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);
        if (queued > 0) {
            ALuint buffers[NX_AudioStream::BufferCount];
            alSourceUnqueueBuffers(stream->source, queued, buffers);
        }
    }

    INX_SeekToStart(*stream);
    INX_GetStreamPlayer().prepareStream(stream);

    if (wasPlaying) {
        alSourcePlay(stream->source);
    }
}

bool NX_IsAudioStreamPlaying(NX_AudioStream* stream)
{
    return stream->isPlaying;
}

bool NX_GetAudioStreamLoop(NX_AudioStream* stream)
{
    return stream->shouldLoop;
}

void NX_SetAudioStreamLoop(NX_AudioStream* stream, bool loop)
{
    stream->shouldLoop = loop;
}

float NX_GetAudioStreamDuration(const NX_AudioStream* stream)
{
    switch (stream->audioFormat) {
    case INX_AudioFormat::WAV: {
        uint64_t totalFrames = stream->decoder.wav->totalPCMFrameCount;
        return static_cast<float>(totalFrames) / stream->decoder.wav->sampleRate;
    }
    case INX_AudioFormat::FLAC: {
        uint64_t totalFrames = stream->decoder.flac->totalPCMFrameCount;
        return static_cast<float>(totalFrames) / stream->decoder.flac->sampleRate;
    }
    case INX_AudioFormat::MP3: {
        uint64_t totalFrames = stream->decoder.mp3->totalPCMFrameCount;
        return static_cast<float>(totalFrames) / stream->decoder.mp3->sampleRate;
    }
    case INX_AudioFormat::OGG: {
        int sampleRate = stb_vorbis_get_info(stream->decoder.ogg).sample_rate;
        uint32_t totalSamples = stb_vorbis_stream_length_in_samples(stream->decoder.ogg);
        return static_cast<float>(totalSamples) / sampleRate;
    }
    default:
        return 0.0f;
    }
}
