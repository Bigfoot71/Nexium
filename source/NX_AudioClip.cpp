/* NX_AudioClip.cpp -- API definition for Nexium's audio clip module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Filesystem.h>
#include <NX/NX_AudioClip.h>
#include <NX/NX_Log.h>

#include "./Detail/Util/FixedArray.hpp"
#include "./Detail/Util/ObjectPool.hpp"
#include "./INX_AudioFormat.hpp"

#include <stb_vorbis.h>
#include <dr_flac.h>
#include <dr_wav.h>
#include <dr_mp3.h>
#include <al.h>

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

struct NX_AudioClip {
    util::FixedArray<ALuint> sources{};
    ALuint buffer{};
    ~NX_AudioClip();
};

NX_AudioClip::~NX_AudioClip()
{
    if (buffer > 0) {
        for (ALuint source : sources) {
            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING || state == AL_PAUSED) {
                alSourceStop(source);
            }
        }
        alDeleteSources(sources.size(), sources.data());
        if (alIsBuffer(buffer)) {
            alDeleteBuffers(1, &buffer);
        }
    }
}

// ============================================================================
// INTERNAL TYPES
// ============================================================================

struct INX_AudioClip_RawData {
    void* pcmData{};
    size_t pcmDataSize{};
    ALsizei sampleRate{};
    ALenum format{};
};

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

INX_AudioClip_RawData INX_LoadAudioClip_RawData_WAV(const void* data, size_t data_size)
{
    INX_AudioClip_RawData result;
    drwav wav;

    if (!drwav_init_memory(&wav, data, data_size, nullptr)) {
        NX_LOG(E, "AUDIO: Failed to initialize WAV decoder");
        return result;
    }

    if (wav.channels == 1 && wav.bitsPerSample == 16) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (wav.channels == 2 && wav.bitsPerSample == 16) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_LOG(E, "AUDIO: Unsupported WAV format (channels: %u, bits: %u)", wav.channels, wav.bitsPerSample);
        drwav_uninit(&wav);
        return result;
    }

    size_t totalFrames = wav.totalPCMFrameCount;
    size_t bytesPerFrame = wav.channels * (wav.bitsPerSample / 8);
    result.pcmDataSize = totalFrames * bytesPerFrame;
    result.pcmData = SDL_malloc(result.pcmDataSize);
    result.sampleRate = wav.sampleRate;

    if (!result.pcmData) {
        NX_LOG(E, "AUDIO: Failed to allocate memory for PCM data");
        drwav_uninit(&wav);
        result = {};
        return result;
    }

    size_t frames_read = drwav_read_pcm_frames(&wav, totalFrames, result.pcmData);
    drwav_uninit(&wav);

    if (frames_read != totalFrames) {
        NX_LOG(E, "AUDIO: Failed to read all PCM frames");
        NX_Free(result.pcmData);
        result = {};
    }

    return result;
}

INX_AudioClip_RawData INX_LoadAudioClip_RawData_FLAC(const void* data, size_t data_size)
{
    INX_AudioClip_RawData result;
    drflac_uint32 channels;
    drflac_uint32 sampleRate;
    drflac_uint64 total_pcm_frame_count;

    drflac_int16* pcmData = drflac_open_memory_and_read_pcm_frames_s16(
        data, data_size, &channels, &sampleRate, &total_pcm_frame_count, nullptr
    );

    if (!pcmData) {
        NX_LOG(E, "AUDIO: Failed to decode FLAC file");
        return result;
    }

    if (channels == 1) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (channels == 2) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_LOG(E, "AUDIO: Unsupported number of channels (%u) in FLAC file", channels);
        drflac_free(pcmData, nullptr);
        return result;
    }

    result.pcmData = pcmData;
    result.pcmDataSize = total_pcm_frame_count * channels * sizeof(drflac_int16);
    result.sampleRate = sampleRate;

    return result;
}

INX_AudioClip_RawData INX_LoadAudioClip_RawData_MP3(const void* data, size_t data_size)
{
    INX_AudioClip_RawData result;
    drmp3_config config;
    drmp3_uint64 total_pcm_frame_count;

    drmp3_int16* pcmData = drmp3_open_memory_and_read_pcm_frames_s16(
        data, data_size, &config, &total_pcm_frame_count, nullptr
    );

    if (!pcmData) {
        NX_LOG(E, "AUDIO: Failed to decode MP3 file");
        return result;
    }

    if (config.channels == 1) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (config.channels == 2) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_LOG(E, "AUDIO: Unsupported number of channels (%u) in MP3 file", config.channels);
        drmp3_free(pcmData, nullptr);
        return result;
    }

    result.pcmData = pcmData;
    result.pcmDataSize = total_pcm_frame_count * config.channels * sizeof(drmp3_int16);
    result.sampleRate = config.sampleRate;

    return result;
}

INX_AudioClip_RawData INX_LoadAudioClip_RawData_OGG(const void* data, size_t data_size)
{
    INX_AudioClip_RawData result;
    int channels = 0;
    int sampleRate = 0;
    short* pcmData = nullptr;

    int total_samples = stb_vorbis_decode_memory(
        (const unsigned char*)data, (int)data_size, &channels, &sampleRate, &pcmData
    );

    if (total_samples == -1 || !pcmData) {
        NX_LOG(E, "AUDIO: Failed to decode OGG file");
        return result;
    }

    if (channels == 1) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (channels == 2) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_LOG(E, "AUDIO: Unsupported number of channels (%d) in OGG file", channels);
        NX_Free(pcmData);
        return result;
    }

    result.pcmData = pcmData;
    result.pcmDataSize = total_samples * channels * sizeof(short);
    result.sampleRate = sampleRate;

    return result;
}

void INX_DestroyAudioClip_RawData(INX_AudioClip_RawData& rawData)
{
    NX_Free(rawData.pcmData);
}

// ============================================================================
// LOCAL STATE
// ============================================================================

using INX_PoolAudioClip = util::ObjectPool<NX_AudioClip, 128>;

static INX_PoolAudioClip& INX_GetPoolAudioClip()
{
    static INX_PoolAudioClip pool{};
    return pool;
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_AudioClip* NX_LoadAudioClip(const char* filePath, int channelCount)
{
    if (channelCount <= 0) {
        NX_LOG(E, "AUDIO: Invalid channel count %d", channelCount);
        return nullptr;
    }

    if (!filePath) {
        NX_LOG(E, "AUDIO: Null file path");
        return nullptr;
    }

    /* --- Load file data --- */

    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    if (!fileData) {
        NX_LOG(E, "AUDIO: Unable to load file '%s'", filePath);
        return nullptr;
    }

    /* --- Decode according to format --- */

    INX_AudioClip_RawData audioData;
    auto format = INX_GetAudioFormat(static_cast<const uint8_t*>(fileData), fileSize);

    switch (format) {
        case INX_AudioFormat::WAV:
            audioData = INX_LoadAudioClip_RawData_WAV(fileData, fileSize);
            break;
        case INX_AudioFormat::FLAC:
            audioData = INX_LoadAudioClip_RawData_FLAC(fileData, fileSize);
            break;
        case INX_AudioFormat::MP3:
            audioData = INX_LoadAudioClip_RawData_MP3(fileData, fileSize);
            break;
        case INX_AudioFormat::OGG:
            audioData = INX_LoadAudioClip_RawData_OGG(fileData, fileSize);
            break;
        default:
            NX_LOG(E, "AUDIO: Unknown audio format for '%s'", filePath);
            NX_Free(fileData);
            return nullptr;
    }

    NX_Free(fileData);

    if (!audioData.pcmData) {
        NX_LOG(E, "AUDIO: Failed to decode audio file '%s'", filePath);
        return nullptr;
    }

    /* --- Create the OpenAL buffer --- */

    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    if (alGetError() != AL_NO_ERROR) {
        NX_LOG(E, "AUDIO: Could not generate OpenAL buffer");
        INX_DestroyAudioClip_RawData(audioData);
        return nullptr;
    }

    /* --- Load data into the buffer --- */

    alBufferData(buffer, audioData.format, audioData.pcmData, audioData.pcmDataSize, audioData.sampleRate);
    if (alGetError() != AL_NO_ERROR) {
        NX_LOG(E, "AUDIO: Could not buffer data to OpenAL");
        INX_DestroyAudioClip_RawData(audioData);
        alDeleteBuffers(1, &buffer);
        return nullptr;
    }

    /* --- Cleanup PCM data (no longer needed now) --- */

    INX_DestroyAudioClip_RawData(audioData);

    /* --- Create the OpenAL sources --- */

    util::FixedArray<ALuint> sources(channelCount, channelCount);
    alGenSources(channelCount, sources.data());
    if (alGetError() != AL_NO_ERROR) {
        NX_LOG(E, "AUDIO: Could not generate OpenAL sources");
        alDeleteBuffers(1, &buffer);
        return nullptr;
    }

    /* --- Attach buffer to all sources --- */

    for (int i = 0; i < channelCount; i++) {
        alSourcei(sources[i], AL_BUFFER, buffer);
        if (alGetError() != AL_NO_ERROR) {
            NX_LOG(E, "AUDIO: Could not attach buffer to source %d", i);
            alDeleteSources(channelCount, sources.data());
            alDeleteBuffers(1, &buffer);
            return nullptr;
        }
    }

    /* --- Push clip to the object pull and return pointer */

    return INX_GetPoolAudioClip().create(std::move(sources), buffer);
}

void NX_DestroyAudioClip(NX_AudioClip* clip)
{
    INX_GetPoolAudioClip().destroy(clip);
}

int NX_PlayAudioClip(NX_AudioClip* clip, int channel)
{
    channel = std::min(channel, static_cast<int>(clip->sources.size() - 1));

    /* --- Select a free channel if necessary --- */

    if (channel < 0) {
        for (int i = 0; i < clip->sources.size(); i++) {
            ALint state = 0;
            alGetSourcei(clip->sources[i], AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) {
                channel = i;
                break;
            }
        }
        if (channel < 0) {
            return -1;
        }
    }

    /* --- Stop current playback on this channel if any --- */

    ALint state;
    alGetSourcei(clip->sources[channel], AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING || state == AL_PAUSED) {
        alSourceRewind(clip->sources[channel]);
    }

    /* --- Play the sound on the specified channel --- */

    alSourcePlay(clip->sources[channel]);

    /* --- Return used channel --- */

    return channel;
}

void NX_PauseAudioClip(NX_AudioClip* clip, int channel)
{
    if (channel >= static_cast<int>(clip->sources.size())) return;
    if (channel >= 0) alSourcePause(clip->sources[channel]);
    else alSourcePausev(clip->sources.size(), clip->sources.data());
}

void NX_StopAudioClip(NX_AudioClip* clip, int channel)
{
    if (channel >= static_cast<int>(clip->sources.size())) return;
    if (channel >= 0) alSourceStop(clip->sources[channel]);
    else alSourceStopv(clip->sources.size(), clip->sources.data());
}

void NX_RewindAudioStream(NX_AudioClip* clip, int channel)
{
    if (channel >= static_cast<int>(clip->sources.size())) return;
    if (channel >= 0) alSourceRewind(clip->sources[channel]);
    else alSourceRewindv(clip->sources.size(), clip->sources.data());
}

bool NX_IsAudioClipPlaying(NX_AudioClip* clip, int channel)
{
    if (channel >= static_cast<int>(clip->sources.size())) {
        return false;
    }

    if (channel >= 0) {
        ALenum state = 0;
        alGetSourcei(clip->sources[channel], AL_SOURCE_STATE, &state);
        return (state == AL_PLAYING);
    }

    for (ALuint source : clip->sources) {
        ALenum state = 0;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING) return true;
    }

    return false;
}

int NX_GetAudioClipChannelCount(NX_AudioClip* clip)
{
    return clip->sources.size();
}

