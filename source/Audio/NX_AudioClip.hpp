/* NX_AudioClip.hpp -- Implementation of the API for managing audio clips
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_AUDIO_CLIP_HPP
#define NX_AUDIO_CLIP_HPP

#include "../Detail/Util/FixedArray.hpp"
#include "../Core/NX_InternalLog.hpp"

#include <NX/NX_Core.h>

#include <SDL3/SDL_stdinc.h>
#include <stb_vorbis.h>
#include <dr_flac.h>
#include <dr_wav.h>
#include <dr_mp3.h>
#include <al.h>

#include <stdint.h>

/* === Forward Declarations === */

class NX_AudioState;

/* === Declaration === */

class NX_AudioClip {
private:
    friend NX_AudioState;

public:
    NX_AudioClip(const char* filePath, int channelCount);
    ~NX_AudioClip();

    NX_AudioClip(const NX_AudioClip&) = delete;
    NX_AudioClip& operator=(const NX_AudioClip&) = delete;

    NX_AudioClip(NX_AudioClip&&) = delete;
    NX_AudioClip& operator=(NX_AudioClip&&) = delete;

    bool isValid() const;

    int play(int channel);
    void pause(int channel);
    void stop(int channel);
    void rewind(int channel);
    bool isPlaying(int channel) const;
    int getChannelCount() const;

private:
    ALuint mBuffer = 0;
    util::FixedArray<ALuint> mSources;

private:
    struct RawData {
        void* pcmData = nullptr;
        size_t pcmDataSize = 0;
        ALenum format = 0;
        ALsizei sampleRate = 0;
    };

private:
    RawData loadWAV(const void* data, size_t data_size);
    RawData loadFLAC(const void* data, size_t data_size);
    RawData loadMP3(const void* data, size_t data_size);
    RawData loadOGG(const void* data, size_t data_size);

private:
    static void cleanupRawData(RawData& audioData);
};

/* === Public Implementation === */

inline NX_AudioClip::~NX_AudioClip()
{
    if (mBuffer > 0) {
        for (size_t i = 0; i < mSources.size(); i++) {
            ALint state;
            alGetSourcei(mSources[i], AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING || state == AL_PAUSED) {
                alSourceStop(mSources[i]);
            }
        }
        alDeleteSources(mSources.size(), mSources.data());
        if (alIsBuffer(mBuffer)) {
            alDeleteBuffers(1, &mBuffer);
        }
    }
}

inline bool NX_AudioClip::isValid() const
{
    return mBuffer > 0;
}

inline int NX_AudioClip::play(int channel)
{
    channel = NX_MIN(channel, static_cast<int>(mSources.size() - 1));

    /* --- Select a free channel if necessary --- */

    if (channel < 0) {
        for (int i = 0; i < mSources.size(); i++) {
            ALint state = 0;
            alGetSourcei(mSources[i], AL_SOURCE_STATE, &state);
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
    alGetSourcei(mSources[channel], AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING || state == AL_PAUSED) {
        alSourceRewind(mSources[channel]);
    }

    /* --- Play the sound on the specified channel --- */

    alSourcePlay(mSources[channel]);

    /* --- Return used channel --- */

    return channel;
}

inline void NX_AudioClip::pause(int channel)
{
    if (channel >= static_cast<int>(mSources.size())) return;
    if (channel >= 0) alSourcePause(mSources[channel]);
    else alSourcePausev(mSources.size(), mSources.data());
}

inline void NX_AudioClip::stop(int channel)
{
    if (channel >= static_cast<int>(mSources.size())) return;
    if (channel >= 0) alSourceStop(mSources[channel]);
    else alSourceStopv(mSources.size(), mSources.data());
}

inline void NX_AudioClip::rewind(int channel)
{
    if (channel >= static_cast<int>(mSources.size())) return;
    if (channel >= 0) alSourceRewind(mSources[channel]);
    else alSourceRewindv(mSources.size(), mSources.data());
}

inline bool NX_AudioClip::isPlaying(int channel) const
{
    if (channel >= static_cast<int>(mSources.size())) {
        return false;
    }

    if (channel >= 0) {
        ALenum state = 0;
        alGetSourcei(mSources[channel], AL_SOURCE_STATE, &state);
        return (state == AL_PLAYING);
    }

    for (ALuint source : mSources) {
        ALenum state = 0;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING) return true;
    }

    return false;
}

inline int NX_AudioClip::getChannelCount() const
{
    return mSources.size();
}

/* === Private Implementation === */

inline NX_AudioClip::RawData NX_AudioClip::loadWAV(const void* data, size_t data_size)
{
    RawData result;
    drwav wav;

    if (!drwav_init_memory(&wav, data, data_size, nullptr)) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to initialize WAV decoder");
        return result;
    }

    // Determine the format
    if (wav.channels == 1 && wav.bitsPerSample == 16) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (wav.channels == 2 && wav.bitsPerSample == 16) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_INTERNAL_LOG(E, "AUDIO: Unsupported WAV format (channels: %u, bits: %u)", wav.channels, wav.bitsPerSample);
        drwav_uninit(&wav);
        return result;
    }

    // Allocate and read PCM data
    size_t totalFrames = wav.totalPCMFrameCount;
    size_t bytesPerFrame = wav.channels * (wav.bitsPerSample / 8);
    result.pcmDataSize = totalFrames * bytesPerFrame;
    result.pcmData = SDL_malloc(result.pcmDataSize);
    result.sampleRate = wav.sampleRate;

    if (!result.pcmData) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to allocate memory for PCM data");
        drwav_uninit(&wav);
        result = {};
        return result;
    }

    size_t frames_read = drwav_read_pcm_frames(&wav, totalFrames, result.pcmData);
    drwav_uninit(&wav);

    if (frames_read != totalFrames) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to read all PCM frames");
        SDL_free(result.pcmData);
        result = {};
    }

    return result;
}

inline NX_AudioClip::RawData NX_AudioClip::loadFLAC(const void* data, size_t data_size)
{
    RawData result;
    drflac_uint32 channels;
    drflac_uint32 sampleRate;
    drflac_uint64 total_pcm_frame_count;

    drflac_int16* pcmData = drflac_open_memory_and_read_pcm_frames_s16(
        data, data_size, &channels, &sampleRate, &total_pcm_frame_count, nullptr
    );

    if (!pcmData) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to decode FLAC file");
        return result;
    }

    if (channels == 1) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (channels == 2) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_INTERNAL_LOG(E, "AUDIO: Unsupported number of channels (%u) in FLAC file", channels);
        drflac_free(pcmData, nullptr);
        return result;
    }

    result.pcmData = pcmData;
    result.pcmDataSize = total_pcm_frame_count * channels * sizeof(drflac_int16);
    result.sampleRate = sampleRate;

    return result;
}

inline NX_AudioClip::RawData NX_AudioClip::loadMP3(const void* data, size_t data_size)
{
    RawData result;
    drmp3_config config;
    drmp3_uint64 total_pcm_frame_count;

    drmp3_int16* pcmData = drmp3_open_memory_and_read_pcm_frames_s16(
        data, data_size, &config, &total_pcm_frame_count, nullptr
    );

    if (!pcmData) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to decode MP3 file");
        return result;
    }

    if (config.channels == 1) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (config.channels == 2) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_INTERNAL_LOG(E, "AUDIO: Unsupported number of channels (%u) in MP3 file", config.channels);
        drmp3_free(pcmData, nullptr);
        return result;
    }

    result.pcmData = pcmData;
    result.pcmDataSize = total_pcm_frame_count * config.channels * sizeof(drmp3_int16);
    result.sampleRate = config.sampleRate;

    return result;
}

inline NX_AudioClip::RawData NX_AudioClip::loadOGG(const void* data, size_t data_size)
{
    RawData result;
    int channels = 0;
    int sampleRate = 0;
    short* pcmData = nullptr;

    int total_samples = stb_vorbis_decode_memory(
        (const unsigned char*)data, (int)data_size, &channels, &sampleRate, &pcmData
    );

    if (total_samples == -1 || !pcmData) {
        NX_INTERNAL_LOG(E, "AUDIO: Failed to decode OGG file");
        return result;
    }

    if (channels == 1) {
        result.format = AL_FORMAT_MONO16;
    }
    else if (channels == 2) {
        result.format = AL_FORMAT_STEREO16;
    }
    else {
        NX_INTERNAL_LOG(E, "AUDIO: Unsupported number of channels (%d) in OGG file", channels);
        SDL_free(pcmData);
        return result;
    }

    result.pcmData = pcmData;
    result.pcmDataSize = total_samples * channels * sizeof(short);
    result.sampleRate = sampleRate;

    return result;
}

inline void NX_AudioClip::cleanupRawData(RawData& audioData)
{
    if (audioData.pcmData) {
        SDL_free(audioData.pcmData);
        audioData.pcmData = nullptr;
    }
}

#endif // NX_AUDIO_CLIP_HPP
