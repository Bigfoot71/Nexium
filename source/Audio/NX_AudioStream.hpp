/* NX_AudioStream.hpp -- Implementation of the API for managing audio streams
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_AUDIO_STREAM_HPP
#define NX_AUDIO_STREAM_HPP

#include "./NX_AudioFormat.hpp"

#include "../Core/NX_InternalLog.hpp"

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>
#include <stb_vorbis.h>
#include <dr_flac.h>
#include <dr_wav.h>
#include <dr_mp3.h>
#include <al.h>

#include <stdint.h>

/* === Forward Declarations === */

class NX_AudioState;

/* === Declaration === */

class NX_AudioStream {
private:
    friend NX_AudioState;

private:
    static constexpr size_t BufferCount = 3;
    static constexpr size_t BufferSize = ((256 /* frames */) * (32 /* blocks */) * 2 /* ch */ * 2 /* bytes/sample */);

public:
    NX_AudioStream(const char* filePath);
    ~NX_AudioStream();

    NX_AudioStream(const NX_AudioStream&) = delete;
    NX_AudioStream& operator=(const NX_AudioStream&) = delete;
    NX_AudioStream(NX_AudioStream&&) = delete;
    NX_AudioStream& operator=(NX_AudioStream&&) = delete;

    bool isValid() const;

    void play();
    void pause();
    void stop();
    void rewind();
    bool isPlaying() const;
    bool getLoop() const;
    void setLoop(bool loop);
    float getDuration() const;

private:
    // OpenAL resources
    ALuint mSource = 0;
    ALuint mBuffers[BufferCount]{};

    // Audio format info
    NX_AudioFormat mFormat = NX_AudioFormat::Unknown;
    int mChannels = 0;
    int mSampleRate = 0;
    ALenum mAlFormat = 0;

    // Decoder handles
    union {
        drwav wav;
        drflac* flac;
        drmp3 mp3;
        stb_vorbis* ogg;
    } mDecoder{};

    // Audio data and state
    void* mAudioData = nullptr;
    bool mShouldLoop = false;
    bool mIsPaused = false;
    bool mIsPlaying = false;

private:
    bool initDecoder(const void* data, size_t dataSize, NX_AudioFormat format);
    void closeDecoder();
    size_t decodeSamples(void* buffer, size_t samples);
    void seekToStart();
    void prepareBuffers();
    void unqueueAllBuffers();
};

/* === Public Implementation === */

inline bool NX_AudioStream::isValid() const
{
    return mFormat != NX_AudioFormat::Unknown && mSource > 0;
}

inline bool NX_AudioStream::isPlaying() const
{
    return mIsPlaying && !mIsPaused;
}

inline bool NX_AudioStream::getLoop() const
{
    return mShouldLoop;
}

inline void NX_AudioStream::setLoop(bool loop)
{
    mShouldLoop = loop;
}

/* === Private Implementation === */

inline bool NX_AudioStream::initDecoder(const void* data, size_t dataSize, NX_AudioFormat format)
{
    // Copy audio data
    mAudioData = SDL_malloc(dataSize);
    if (!mAudioData) {
        return false;
    }
    SDL_memcpy(mAudioData, data, dataSize);

    mFormat = format;

    switch (format) {
        case NX_AudioFormat::WAV: {
            if (!drwav_init_memory(&mDecoder.wav, mAudioData, dataSize, nullptr)) {
                SDL_free(mAudioData);
                mAudioData = nullptr;
                return false;
            }

            mChannels = mDecoder.wav.channels;
            mSampleRate = mDecoder.wav.sampleRate;
            break;
        }

        case NX_AudioFormat::FLAC: {
            mDecoder.flac = drflac_open_memory(mAudioData, dataSize, nullptr);
            if (!mDecoder.flac) {
                SDL_free(mAudioData);
                mAudioData = nullptr;
                return false;
            }

            mChannels = mDecoder.flac->channels;
            mSampleRate = mDecoder.flac->sampleRate;
            break;
        }

        case NX_AudioFormat::MP3: {
            if (!drmp3_init_memory(&mDecoder.mp3, mAudioData, dataSize, nullptr)) {
                SDL_free(mAudioData);
                mAudioData = nullptr;
                return false;
            }

            mChannels = mDecoder.mp3.channels;
            mSampleRate = mDecoder.mp3.sampleRate;
            break;
        }

        case NX_AudioFormat::OGG: {
            int error;
            mDecoder.ogg = stb_vorbis_open_memory(
                static_cast<const unsigned char*>(mAudioData), dataSize, &error, nullptr
            );
            if (!mDecoder.ogg) {
                SDL_free(mAudioData);
                mAudioData = nullptr;
                return false;
            }

            stb_vorbis_info info = stb_vorbis_get_info(mDecoder.ogg);
            mChannels = info.channels;
            mSampleRate = info.sample_rate;
            break;
        }

        default:
            SDL_free(mAudioData);
            mAudioData = nullptr;
            return false;
    }

    // Determine OpenAL format
    if (mChannels == 1) {
        mAlFormat = AL_FORMAT_MONO16;
    }
    else if (mChannels == 2) {
        mAlFormat = AL_FORMAT_STEREO16;
    }
    else {
        NX_INTERNAL_LOG(E, "AUDIO: Unsupported number of channels (%d)", mChannels);
        closeDecoder();
        return false;
    }

    return true;
}

inline void NX_AudioStream::closeDecoder()
{
    switch (mFormat) {
    case NX_AudioFormat::WAV:
        drwav_uninit(&mDecoder.wav);
        break;
    case NX_AudioFormat::FLAC:
        if (mDecoder.flac) {
            drflac_close(mDecoder.flac);
            mDecoder.flac = nullptr;
        }
        break;
    case NX_AudioFormat::MP3:
        drmp3_uninit(&mDecoder.mp3);
        break;
    case NX_AudioFormat::OGG:
        if (mDecoder.ogg) {
            stb_vorbis_close(mDecoder.ogg);
            mDecoder.ogg = nullptr;
        }
        break;
    default:
        break;
    }

    if (mAudioData) {
        SDL_free(mAudioData);
        mAudioData = nullptr;
    }

    mFormat = NX_AudioFormat::Unknown;
}

inline size_t NX_AudioStream::decodeSamples(void* buffer, size_t samples)
{
    switch (mFormat) {
    case NX_AudioFormat::WAV:
        return drwav_read_pcm_frames_s16(&mDecoder.wav, samples, static_cast<drwav_int16*>(buffer));
    case NX_AudioFormat::FLAC:
        return drflac_read_pcm_frames_s16(mDecoder.flac, samples, static_cast<drflac_int16*>(buffer));
    case NX_AudioFormat::MP3:
        return drmp3_read_pcm_frames_s16(&mDecoder.mp3, samples, static_cast<drmp3_int16*>(buffer));
    case NX_AudioFormat::OGG:
        return stb_vorbis_get_samples_short_interleaved(
            mDecoder.ogg, mChannels, static_cast<short*>(buffer), samples * mChannels
        );
    default:
        return 0;
    }
}

inline void NX_AudioStream::seekToStart()
{
    switch (mFormat) {
    case NX_AudioFormat::WAV:
        drwav_seek_to_pcm_frame(&mDecoder.wav, 0);
        break;
    case NX_AudioFormat::FLAC:
        drflac_seek_to_pcm_frame(mDecoder.flac, 0);
        break;
    case NX_AudioFormat::MP3:
        drmp3_seek_to_pcm_frame(&mDecoder.mp3, 0);
        break;
    case NX_AudioFormat::OGG:
        stb_vorbis_seek(mDecoder.ogg, 0);
        break;
    default:
        break;
    }
}

inline void NX_AudioStream::unqueueAllBuffers()
{
    ALuint buffersToRemove[BufferCount];

    /* --- Unqueue all processed buffers --- */

    ALint processed = 0;
    alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
    SDL_assert(processed <= BufferCount);
    if (processed > 0) {
        alSourceUnqueueBuffers(mSource, processed, buffersToRemove);
    }

    /* --- Unqueue any remaining buffer --- */

    ALint queued = 0;
    alGetSourcei(mSource, AL_BUFFERS_QUEUED, &queued);
    SDL_assert(queued <= BufferCount);
    if (queued > 0) {
        alSourceUnqueueBuffers(mSource, queued, buffersToRemove);
    }
}

#endif // NX_AUDIO_STREAM_HPP
