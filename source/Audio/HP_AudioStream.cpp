/* HP_AudioStream.cpp -- Implementation of the API for managing audio streams
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./HP_AudioStream.hpp"
#include "./HP_AudioState.hpp"

#include "../Core/HP_InternalLog.hpp"

/* === Public Implementation === */

HP_AudioStream::HP_AudioStream(const char* filePath)
{
    if (!filePath) {
        HP_INTERNAL_LOG(E, "AUDIO: File path is null");
        return;
    }

    /* --- Load the file data --- */

    size_t fileSize = 0;
    void* fileData = HP_LoadFile(filePath, &fileSize);
    if (!fileData) {
        HP_INTERNAL_LOG(E, "AUDIO: Failed to load music file: %s", filePath);
        return;
    }

    /* --- Determine the format and initialize the decoder --- */

    HP_AudioFormat format = getAudioFormat(static_cast<const uint8_t*>(fileData), fileSize);
    if (format == HP_AudioFormat::Unknown) {
        HP_INTERNAL_LOG(E, "AUDIO: Unknown or unsupported audio format in file: %s", filePath);
        SDL_free(fileData);
        return;
    }

    if (!initDecoder(fileData, fileSize, format)) {
        HP_INTERNAL_LOG(E, "AUDIO: Failed to initialize decoder for file: %s", filePath);
        SDL_free(fileData);
        return;
    }

    SDL_free(fileData);

    /* --- Create OpenAL buffers --- */

    alGenBuffers(BufferCount, mBuffers);
    if (alGetError() != AL_NO_ERROR) {
        HP_INTERNAL_LOG(E, "AUDIO: Failed to generate OpenAL buffers for music");
        closeDecoder();
        return;
    }

    /* --- Create the OpenAL source --- */

    alGenSources(1, &mSource);
    if (alGetError() != AL_NO_ERROR) {
        HP_INTERNAL_LOG(E, "AUDIO: Failed to generate OpenAL source for music");
        alDeleteBuffers(BufferCount, mBuffers);
        closeDecoder();
        return;
    }

    /* --- Pre-fill buffers --- */

    prepareBuffers();

    /* --- Set initial volume --- */

    alSourcef(mSource, AL_GAIN, gAudio->getStreamGain());
}

HP_AudioStream::~HP_AudioStream()
{
    if (mSource > 0)
    {
        /* --- Remove from active streams and stop playback --- */

        // NOTE: The HP_AudioStream destructor can be called during the destruction
        //       of gAudio, at which point the pointer may be null. In that case,
        //       there is no need to remove the stream from the global state,
        //       as everything will be cleaned up anyway.

        if (gAudio) {
            gAudio->removeActiveStream(this);
        }

        /* --- Clean up OpenAL resources --- */

        alDeleteSources(1, &mSource);
        alDeleteBuffers(BufferCount, mBuffers);
    }

    /* --- Clean the decoder --- */

    closeDecoder();
}

void HP_AudioStream::play()
{
    if (!isValid()) {
        return;
    }

    if (mIsPaused && mIsPlaying) {
        alSourcePlay(mSource);
        mIsPaused = false;
        return;
    }

    gAudio->addActiveStream(this);
    alSourcePlay(mSource);
    mIsPaused = false;
    mIsPlaying = true;
}

void HP_AudioStream::pause()
{
    if (mIsPlaying && !mIsPaused) {
        alSourcePause(mSource);
        mIsPaused = true;
    }
}

void HP_AudioStream::stop()
{
    if (!mIsPlaying) {
        return;
    }

    gAudio->removeActiveStream(this);
    alSourceStop(mSource);
    unqueueAllBuffers();

    seekToStart();
    mIsPaused = false;
    mIsPlaying = false;

    prepareBuffers(); //< pre-fill buffers for next playback
}

void HP_AudioStream::rewind()
{
    bool wasPlaying = (mIsPlaying && !mIsPaused);

    if (mIsPlaying) {
        alSourceStop(mSource);
        unqueueAllBuffers();
    }

    seekToStart();
    prepareBuffers();

    if (wasPlaying) {
        alSourcePlay(mSource);
    }
}

float HP_AudioStream::getDuration() const
{
    if (!isValid()) {
        return 0.0f;
    }

    switch (mFormat) {
    case HP_AudioFormat::WAV: {
        uint64_t totalFrames = mDecoder.wav.totalPCMFrameCount;
        return static_cast<float>(totalFrames) / static_cast<float>(mSampleRate);
    }
    case HP_AudioFormat::FLAC: {
        uint64_t totalFrames = mDecoder.flac->totalPCMFrameCount;
        return static_cast<float>(totalFrames) / static_cast<float>(mSampleRate);
    }
    case HP_AudioFormat::MP3: {
        uint64_t totalFrames = mDecoder.mp3.totalPCMFrameCount;
        return static_cast<float>(totalFrames) / static_cast<float>(mSampleRate);
    }
    case HP_AudioFormat::OGG: {
        uint32_t totalSamples = stb_vorbis_stream_length_in_samples(mDecoder.ogg);
        return static_cast<float>(totalSamples) / static_cast<float>(mSampleRate);
    }
    default:
        break;
    }

    return 0.0f;
}

/* === Private Implementation === */

void HP_AudioStream::prepareBuffers()
{
    for (int i = 0; i < BufferCount; i++)
    {
        size_t samplesToRead = BufferSize / (mChannels * sizeof(int16_t));

        // Request decode buffer from global pool
        uint8_t* decodeBuffer = gAudio->requestDecodeBuffer();
        if (!decodeBuffer) {
            break;
        }

        size_t samplesRead = decodeSamples(decodeBuffer, samplesToRead);
        if (samplesRead == 0) {
            gAudio->releaseDecodeBuffer(decodeBuffer);
            break;
        }

        size_t dataSize = samplesRead * mChannels * sizeof(int16_t);
        alBufferData(
            mBuffers[i], mAlFormat,
            static_cast<void*>(decodeBuffer), dataSize,
            mSampleRate
        );
        alSourceQueueBuffers(mSource, 1, &mBuffers[i]);

        gAudio->releaseDecodeBuffer(decodeBuffer);
    }
}