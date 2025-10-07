/* NX_AudioState.cpp -- Contains the global state of the module and everything needed for audio decoding
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_AUDIO_STATE_HPP
#define NX_AUDIO_STATE_HPP

#include "../Detail/Util/DynamicArray.hpp"
#include "../Detail/Util/ObjectPool.hpp"
#include "../Audio/NX_AudioStream.hpp"
#include "../Audio/NX_AudioClip.hpp"

#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_mutex.h>
#include <alc.h>
#include <al.h>

#include <memory>
#include <array>

/* === Global State === */

extern std::unique_ptr<class NX_AudioState> gAudio;

/* === Declaration === */

class NX_AudioState {
public:
    /* --- Public Methods --- */

    float getMasterVolume();
    float getClipVolume();
    float getStreamVolume();

    void setMasterVolume(float volume);
    void setClipVolume(float volume);
    void setStreamVolume(float volume);

    float getClipGain() const;
    float getStreamGain() const;

    NX_AudioClip* createClip(const char* filePath, int channelCount);
    void destroyClip(NX_AudioClip* clip);

    NX_AudioStream* createStream(const char* filePath);
    void destroyStream(NX_AudioStream* stream);

    // Stream management
    void addActiveStream(NX_AudioStream* stream);
    void removeActiveStream(NX_AudioStream* stream);

    // Buffer pool management
    uint8_t* requestDecodeBuffer();
    void releaseDecodeBuffer(uint8_t* buffer);

public:
    /* --- Constructors --- */

    NX_AudioState();
    ~NX_AudioState();

private:
    /* --- Thread Function --- */

    static int updateStreamThread(void* state);
    void updateStreams();

    /* --- Private Members --- */

    // OpenAL handlers
    ALCcontext* mALContext;
    ALCdevice* mALDevice;

    // Object pools
    util::ObjectPool<NX_AudioClip, 64> mClips;
    util::ObjectPool<NX_AudioStream, 16> mStreams;

    // Volume controls (0.0 to 1.0)
    float mVolumeMaster;
    float mVolumeClips;
    float mVolumeStreams;

    // Streaming thread
    SDL_Thread* mStreamThread;
    SDL_Mutex* mStreamThreadMutex;
    SDL_Condition* mStreamThreadCondition;
    SDL_AtomicInt mStreamThreadShouldStop;

    // Active streams
    util::DynamicArray<NX_AudioStream*> mActiveStreams;

    // Global decode buffer pool
    static constexpr size_t MaxDecodeBuffers = 32;
    static constexpr size_t DecodeBufferSize = ((256 /* frames */) * (32 /* blocks */) * 2 /* ch */ * 2 /* bytes/sample */);

    std::array<uint8_t*, MaxDecodeBuffers> mDecodeBuffers{};
    util::DynamicArray<bool> mBufferAvailable;
    size_t mActiveBufferCount;

private:
    /* --- Buffer Pool Methods --- */

    void initDecodeBufferPool();
    void cleanupDecodeBufferPool();

};

/* === Public Implementation === */

inline float NX_AudioState::getMasterVolume()
{
    return mVolumeMaster;
}

inline float NX_AudioState::getClipVolume()
{
    return mVolumeClips;
}

inline float NX_AudioState::getStreamVolume()
{
    return mVolumeStreams;
}

inline NX_AudioClip* NX_AudioState::createClip(const char* filePath, int channelCount)
{
    NX_AudioClip* clip = mClips.create(filePath, channelCount);
    if (clip != nullptr && !clip->isValid()) {
        mClips.destroy(clip);
        clip = nullptr;
    }
    return clip;
}

inline void NX_AudioState::destroyClip(NX_AudioClip* clip)
{
    mClips.destroy(clip);
}

inline NX_AudioStream* NX_AudioState::createStream(const char* filePath)
{
    NX_AudioStream* stream = mStreams.create(filePath);
    if (stream != nullptr && !stream->isValid()) {
        mStreams.destroy(stream);
        stream = nullptr;
    }
    return stream;
}

inline void NX_AudioState::destroyStream(NX_AudioStream* stream)
{
    removeActiveStream(stream);
    mStreams.destroy(stream);
}

inline uint8_t* NX_AudioState::requestDecodeBuffer()
{
    SDL_LockMutex(mStreamThreadMutex);

    for (size_t i = 0; i < MaxDecodeBuffers; ++i) {
        if (mBufferAvailable[i]) {
            mBufferAvailable[i] = false;
            mActiveBufferCount++;
            SDL_UnlockMutex(mStreamThreadMutex);
            return mDecodeBuffers[i];
        }
    }

    SDL_UnlockMutex(mStreamThreadMutex);
    return nullptr; // No buffer available
}

inline void NX_AudioState::releaseDecodeBuffer(uint8_t* buffer)
{
    if (!buffer) return;

    SDL_LockMutex(mStreamThreadMutex);

    for (size_t i = 0; i < MaxDecodeBuffers; ++i) {
        if (mDecodeBuffers[i] == buffer) {
            mBufferAvailable[i] = true;
            mActiveBufferCount--;
            break;
        }
    }

    SDL_UnlockMutex(mStreamThreadMutex);
}

#endif // NX_AUDIO_STATE_HPP
