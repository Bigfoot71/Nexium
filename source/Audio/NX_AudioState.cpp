/* NX_AudioState.hpp -- Contains the global state of the module and everything needed for audio decoding
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_AudioState.hpp"

#include "../Core/NX_InternalLog.hpp"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_timer.h>
#include <algorithm>
#include <stdexcept>    //< Only for initialization errors

/* === DR WAV Implementation === */

#define DR_WAV_IMPLEMENTATION

#define DRWAV_ASSERT(expression)           SDL_assert(expression)
#define DRWAV_MALLOC(sz)                   SDL_malloc((sz))
#define DRWAV_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRWAV_FREE(p)                      SDL_free((p))
#define DRWAV_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRWAV_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_wav.h>

/* === DR Flac Implementation === */

#define DR_FLAC_IMPLEMENTATION

#define DRFLAC_ASSERT(expression)           SDL_assert(expression)
#define DRFLAC_MALLOC(sz)                   SDL_malloc((sz))
#define DRFLAC_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRFLAC_FREE(p)                      SDL_free((p))
#define DRFLAC_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRFLAC_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_flac.h>

/* === DR MP3 Implementation === */

#define DR_MP3_IMPLEMENTATION

#define DRMP3_ASSERT(expression)           SDL_assert(expression)
#define DRMP3_MALLOC(sz)                   SDL_malloc((sz))
#define DRMP3_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRMP3_FREE(p)                      SDL_free((p))
#define DRMP3_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRMP3_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_mp3.h>

/* === STB Vorbis Implementation === */

#undef STB_VORBIS_HEADER_ONLY

#define STB_VORBIS_MALLOC(sz)   SDL_malloc(sz)
#define STB_VORBIS_FREE(p)      SDL_free(p)

#include <stb_vorbis.c>

/* === Local Helper Functions */

namespace {

inline float convertLinearToLog(float linearVolume)
{
    if (linearVolume <= 0.0f) return 0.0f;
    if (linearVolume >= 1.0f) return 1.0f;

    return powf(linearVolume, 3.0f);
}

inline float convertLogToLinear(float logVolume)
{
    if (logVolume <= 0.0f) return 0.0f;
    if (logVolume >= 1.0f) return 1.0f;

    return powf(logVolume, 1.0f / 3.0f);
}

} // namespace

/* === Public Methods === */

void NX_AudioState::setMasterVolume(float volume)
{
    mVolumeMaster = NX_CLAMP(volume, 0.0f, 1.0f);

    /* --- Update clip volumes --- */

    float clipGain = getClipGain();

    for (NX_AudioClip& clip : mClips) {
        for (ALuint source : clip.mSources) {
            alSourcef(source, AL_GAIN, clipGain);
        }
    }

    /* --- Update stream volume --- */

    float streamGain = getStreamGain();

    for (NX_AudioStream& stream : mStreams) {
        alSourcef(stream.mSource, AL_GAIN, streamGain);
    }
}

void NX_AudioState::setClipVolume(float volume)
{
    mVolumeClips = NX_CLAMP(volume, 0.0f, 1.0f);

    /* --- Update clip volume --- */

    float clipGain = getClipGain();

    for (NX_AudioClip& clip : mClips) {
        for (ALuint source : clip.mSources) {
            alSourcef(source, AL_GAIN, clipGain);
        }
    }
}

void NX_AudioState::setStreamVolume(float volume)
{
    mVolumeStreams = volume;

    /* --- Update stream volume --- */

    float streamGain = getStreamGain();

    for (NX_AudioStream& stream : mStreams) {
        alSourcef(stream.mSource, AL_GAIN, streamGain);
    }
}

float NX_AudioState::getClipGain() const
{
    float masterGain = convertLinearToLog(mVolumeMaster);
    float clipGain = convertLinearToLog(mVolumeClips);
    return masterGain * clipGain;
}

float NX_AudioState::getStreamGain() const
{
    float masterGain = convertLinearToLog(mVolumeMaster);
    float streamGain = convertLinearToLog(mVolumeStreams);
    return masterGain * streamGain;
}

void NX_AudioState::addActiveStream(NX_AudioStream* stream)
{
    if (!stream) return;

    SDL_LockMutex(mStreamThreadMutex);

    // Check if already in list
    auto it = std::find(mActiveStreams.begin(), mActiveStreams.end(), stream);
    if (it == mActiveStreams.end()) {
        mActiveStreams.push_back(stream);
        SDL_SignalCondition(mStreamThreadCondition);
    }

    SDL_UnlockMutex(mStreamThreadMutex);
}

void NX_AudioState::removeActiveStream(NX_AudioStream* stream)
{
    if (!stream) return;

    SDL_LockMutex(mStreamThreadMutex);

    auto it = std::find(mActiveStreams.begin(), mActiveStreams.end(), stream);
    if (it != mActiveStreams.end()) {
        mActiveStreams.erase(it);
    }

    SDL_UnlockMutex(mStreamThreadMutex);
}

/* === Constructors === */

NX_AudioState::NX_AudioState()
    : mALDevice(nullptr)
    , mALContext(nullptr)
    , mVolumeMaster(1.0f)
    , mVolumeClips(1.0f)
    , mVolumeStreams(1.0f)
    , mStreamThread(nullptr)
    , mStreamThreadMutex(nullptr)
    , mStreamThreadCondition(nullptr)
    , mActiveBufferCount(0)
{
    /* --- Create the OpenAL context and device --- */

    mALDevice = alcOpenDevice(nullptr);
    if (mALDevice == nullptr) {
        throw std::runtime_error(std::string("AUDIO: Failed to create OpenAL device; ") + SDL_GetError());
    }

    mALContext = alcCreateContext(mALDevice, nullptr);
    if (mALContext == nullptr) {
        alcCloseDevice(mALDevice);
        throw std::runtime_error(std::string("AUDIO: Failed to create OpenAL context; ") + SDL_GetError());
    }

    alcMakeContextCurrent(mALContext);

    /* --- Initialize decode buffer pool --- */

    initDecodeBufferPool();

    /* --- Initialize streaming thread --- */

    // Create mutex and condition variable
    mStreamThreadMutex = SDL_CreateMutex();
    if (mStreamThreadMutex == nullptr) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to create stream thread mutex");
        cleanupDecodeBufferPool();
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(mALContext);
        alcCloseDevice(mALDevice);
        throw std::runtime_error("AUDIO: Failed to create stream thread mutex");
    }

    mStreamThreadCondition = SDL_CreateCondition();
    if (mStreamThreadCondition == nullptr) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to create stream thread condition");
        SDL_DestroyMutex(mStreamThreadMutex);
        cleanupDecodeBufferPool();
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(mALContext);
        alcCloseDevice(mALDevice);
        throw std::runtime_error("AUDIO: Failed to create stream thread condition");
    }

    // Initialize atomic flag and clear active streams
    SDL_SetAtomicInt(&mStreamThreadShouldStop, 0);
    mActiveStreams.clear();

    // Create and start the streaming thread
    mStreamThread = SDL_CreateThread(updateStreamThread, "AudioStreamThread", this);
    if (mStreamThread == nullptr) {
        NX_INTERNAL_LOG(W, "AUDIO: Failed to create stream streaming thread");
        SDL_DestroyCondition(mStreamThreadCondition);
        SDL_DestroyMutex(mStreamThreadMutex);
        mStreamThreadCondition = nullptr;
        mStreamThreadMutex = nullptr;
        // Continue without streaming thread - we can still play clips
    }
}

NX_AudioState::~NX_AudioState()
{
    /* --- Shutdown the stream thread FIRST --- */

    if (mStreamThread) {
        // Signal the thread to stop
        SDL_SetAtomicInt(&mStreamThreadShouldStop, 1);

        // Wake up the thread if it's waiting
        if (mStreamThreadCondition) {
            SDL_LockMutex(mStreamThreadMutex);
            mActiveStreams.clear();
            SDL_SignalCondition(mStreamThreadCondition);
            SDL_UnlockMutex(mStreamThreadMutex);
        }

        // Wait for thread to finish
        SDL_WaitThread(mStreamThread, nullptr);
        mStreamThread = nullptr;
    }

    /* --- Clean up synchronization primitives --- */

    if (mStreamThreadCondition) {
        SDL_DestroyCondition(mStreamThreadCondition);
        mStreamThreadCondition = nullptr;
    }

    if (mStreamThreadMutex) {
        SDL_DestroyMutex(mStreamThreadMutex);
        mStreamThreadMutex = nullptr;
    }

    /* --- NOW it's safe to clear object pools --- */

    mStreams.clear();
    mClips.clear();

    /* --- Clean up decode buffer pool --- */

    cleanupDecodeBufferPool();

    /* --- Close OpenAL device and context --- */

    if (mALContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(mALContext);
        mALContext = nullptr;
    }

    if (mALDevice) {
        alcCloseDevice(mALDevice);
        mALDevice = nullptr;
    }
}

/* === Private Methods === */

void NX_AudioState::initDecodeBufferPool()
{
    mBufferAvailable.resize(MaxDecodeBuffers, true);
    mActiveBufferCount = 0;

    for (size_t i = 0; i < MaxDecodeBuffers; ++i) {
        mDecodeBuffers[i] = static_cast<uint8_t*>(SDL_malloc(DecodeBufferSize));
        if (!mDecodeBuffers[i]) {
            NX_INTERNAL_LOG(E, "AUDIO: Failed to allocate decode buffer %zu", i);
            // Clean up already allocated buffers
            for (size_t j = 0; j < i; ++j) {
                SDL_free(mDecodeBuffers[j]);
                mDecodeBuffers[j] = nullptr;
            }
            throw std::runtime_error("AUDIO: Failed to initialize decode buffer pool");
        }
    }
}

void NX_AudioState::cleanupDecodeBufferPool()
{
    for (size_t i = 0; i < MaxDecodeBuffers; ++i) {
        if (mDecodeBuffers[i]) {
            SDL_free(mDecodeBuffers[i]);
            mDecodeBuffers[i] = nullptr;
        }
    }
    mBufferAvailable.clear();
}

int NX_AudioState::updateStreamThread(void* state)
{
    NX_AudioState& self = *static_cast<NX_AudioState*>(state);

    while (!SDL_GetAtomicInt(&self.mStreamThreadShouldStop))
    {
        SDL_LockMutex(self.mStreamThreadMutex);

        /* --- Wait for work or shutdown signal --- */

        while (self.mActiveStreams.empty() && !SDL_GetAtomicInt(&self.mStreamThreadShouldStop)) {
            SDL_WaitCondition(self.mStreamThreadCondition, self.mStreamThreadMutex);
        }

        /* --- Check if we should exit --- */

        if (SDL_GetAtomicInt(&self.mStreamThreadShouldStop)) {
            SDL_UnlockMutex(self.mStreamThreadMutex);
            break;
        }

        /* --- Update all active streams --- */

        self.updateStreams();

        SDL_UnlockMutex(self.mStreamThreadMutex);

        /* --- Sleep for a short time to avoid busy waiting --- */

        SDL_Delay(16); // ~60 FPS
    }

    return 0;
}

void NX_AudioState::updateStreams()
{
    // Process streams in reverse order so we can safely remove during iteration
    for (auto it = mActiveStreams.rbegin(); it != mActiveStreams.rend(); ++it)
    {
        NX_AudioStream* stream = *it;
        if (!stream || !stream->isValid()) {
            continue;
        }

        /* --- Skip paused streams --- */

        if (stream->mIsPaused) {
            continue;
        }

        /* --- Check OpenAL source state --- */

        ALint sourceState;
        alGetSourcei(stream->mSource, AL_SOURCE_STATE, &sourceState);

        /* --- Check how many buffers have been processed --- */

        ALint processed = 0;
        alGetSourcei(stream->mSource, AL_BUFFERS_PROCESSED, &processed);

        /* --- Process completed buffers --- */

        bool endOfStream = false;

        while (processed > 0 && !SDL_GetAtomicInt(&mStreamThreadShouldStop))
        {
            ALuint buffer;
            alSourceUnqueueBuffers(stream->mSource, 1, &buffer);

            // Request decode buffer from pool
            uint8_t* decodeBuffer = requestDecodeBuffer();
            if (!decodeBuffer) {
                // No buffer available, skip this update
                processed--;
                continue;
            }

            // Calculate decode size
            size_t samplesToRead = DecodeBufferSize / (stream->mChannels * sizeof(int16_t));
            size_t samplesRead = stream->decodeSamples(decodeBuffer, samplesToRead);

            if (samplesRead > 0) {
                // Successfully decoded data
                size_t dataSize = samplesRead * stream->mChannels * sizeof(int16_t);
                alBufferData(
                    buffer, stream->mAlFormat, decodeBuffer,
                    dataSize, stream->mSampleRate
                );
                alSourceQueueBuffers(stream->mSource, 1, &buffer);
            }
            else {
                // End of file reached
                if (stream->mShouldLoop) {
                    // Loop back to beginning and try decoding again
                    stream->seekToStart();

                    samplesRead = stream->decodeSamples(decodeBuffer, samplesToRead);

                    if (samplesRead > 0) {
                        size_t dataSize = samplesRead * stream->mChannels * sizeof(int16_t);
                        alBufferData(
                            buffer, stream->mAlFormat, decodeBuffer,
                            dataSize, stream->mSampleRate
                        );
                        alSourceQueueBuffers(stream->mSource, 1, &buffer);
                    }
                    else {
                        // Cannot decode even after rewind
                        endOfStream = true;
                    }
                }
                else {
                    // No loop, mark as finished
                    endOfStream = true;
                }
            }

            releaseDecodeBuffer(decodeBuffer);
            processed--;
        }

        /* --- Check if the source has stopped naturally --- */

        ALint queued = 0;
        alGetSourcei(stream->mSource, AL_BUFFERS_QUEUED, &queued);

        if (queued == 0 && endOfStream) {
            // No more buffers and end of file reached - stop this stream
            auto forwardIt = std::find(mActiveStreams.begin(), mActiveStreams.end(), stream);
            if (forwardIt != mActiveStreams.end()) {
                mActiveStreams.erase(forwardIt);
            }

            // Reset stream to beginning for future playback
            stream->seekToStart();
            stream->mIsPlaying = false;

            // Pre-fill buffers for next playback
            stream->prepareBuffers();
        }
        else if (sourceState != AL_PLAYING && !stream->mIsPaused && queued > 0) {
            // Source stopped unexpectedly, restart it
            NX_INTERNAL_LOG(W, "AUDIO: Stream source stopped unexpectedly, restarting...");
            alSourcePlay(stream->mSource);
        }
    }
}
