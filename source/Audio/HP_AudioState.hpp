/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef HP_AUDIO_STATE_HPP
#define HP_AUDIO_STATE_HPP

#include "../Detail/Util/DynamicArray.hpp"
#include "../Detail/Util/ObjectPool.hpp"
#include "../Audio/HP_AudioStream.hpp"
#include "../Audio/HP_AudioClip.hpp"

#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_mutex.h>
#include <alc.h>
#include <al.h>

#include <memory>
#include <array>

/* === Global State === */

extern std::unique_ptr<class HP_AudioState> gAudio;

/* === Declaration === */

class HP_AudioState {
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

    HP_AudioClip* createClip(const char* filePath, int channelCount);
    void destroyClip(HP_AudioClip* clip);

    HP_AudioStream* createStream(const char* filePath);
    void destroyStream(HP_AudioStream* stream);

    // Stream management
    void addActiveStream(HP_AudioStream* stream);
    void removeActiveStream(HP_AudioStream* stream);

    // Buffer pool management
    uint8_t* requestDecodeBuffer();
    void releaseDecodeBuffer(uint8_t* buffer);

public:
    /* --- Constructors --- */

    HP_AudioState();
    ~HP_AudioState();

private:
    /* --- Thread Function --- */

    static int updateStreamThread(void* state);
    void updateStreams();

    /* --- Private Members --- */

    // OpenAL handlers
    ALCcontext* mALContext;
    ALCdevice* mALDevice;

    // Object pools
    util::ObjectPool<HP_AudioClip, 64> mClips;
    util::ObjectPool<HP_AudioStream, 16> mStreams;

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
    util::DynamicArray<HP_AudioStream*> mActiveStreams;

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

inline float HP_AudioState::getMasterVolume()
{
    return mVolumeMaster;
}

inline float HP_AudioState::getClipVolume()
{
    return mVolumeClips;
}

inline float HP_AudioState::getStreamVolume()
{
    return mVolumeStreams;
}

inline HP_AudioClip* HP_AudioState::createClip(const char* filePath, int channelCount)
{
    HP_AudioClip* clip = mClips.create(filePath, channelCount);
    if (clip != nullptr && !clip->isValid()) {
        mClips.destroy(clip);
        clip = nullptr;
    }
    return clip;
}

inline void HP_AudioState::destroyClip(HP_AudioClip* clip)
{
    mClips.destroy(clip);
}

inline HP_AudioStream* HP_AudioState::createStream(const char* filePath)
{
    HP_AudioStream* stream = mStreams.create(filePath);
    if (stream != nullptr && !stream->isValid()) {
        mStreams.destroy(stream);
        stream = nullptr;
    }
    return stream;
}

inline void HP_AudioState::destroyStream(HP_AudioStream* stream)
{
    removeActiveStream(stream);
    mStreams.destroy(stream);
}

inline uint8_t* HP_AudioState::requestDecodeBuffer()
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

inline void HP_AudioState::releaseDecodeBuffer(uint8_t* buffer)
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

#endif // HP_AUDIO_STATE_HPP
