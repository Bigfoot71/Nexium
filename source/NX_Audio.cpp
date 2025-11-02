/* NX_Audio.cpp -- API definition for Nexium's audio module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Audio.hpp"
#include <NX/NX_Log.h>

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <cmath>
#include <alc.h>
#include <al.h>

// ============================================================================
// DRWAV IMPLEMENTATION
// ============================================================================

#define DR_WAV_IMPLEMENTATION

#define DRWAV_ASSERT(expression)           SDL_assert(expression)
#define DRWAV_MALLOC(sz)                   SDL_malloc((sz))
#define DRWAV_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRWAV_FREE(p)                      SDL_free((p))
#define DRWAV_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRWAV_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_wav.h>

// ============================================================================
// DRFLAC IMPLEMENTATION
// ============================================================================

#define DR_FLAC_IMPLEMENTATION

#define DRFLAC_ASSERT(expression)           SDL_assert(expression)
#define DRFLAC_MALLOC(sz)                   SDL_malloc((sz))
#define DRFLAC_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRFLAC_FREE(p)                      SDL_free((p))
#define DRFLAC_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRFLAC_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_flac.h>

// ============================================================================
// DRMP3 IMPLEMENTATION
// ============================================================================

#define DR_MP3_IMPLEMENTATION

#define DRMP3_ASSERT(expression)           SDL_assert(expression)
#define DRMP3_MALLOC(sz)                   SDL_malloc((sz))
#define DRMP3_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRMP3_FREE(p)                      SDL_free((p))
#define DRMP3_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRMP3_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_mp3.h>

// ============================================================================
// STB VORBIS IMPLEMENTATION
// ============================================================================

#undef STB_VORBIS_HEADER_ONLY

#define STB_VORBIS_MALLOC(sz)   SDL_malloc(sz)
#define STB_VORBIS_FREE(p)      SDL_free(p)

#include <stb_vorbis.c>

// ============================================================================
// LOCAL STATE
// ============================================================================

static struct INX_AudioState {
    ALCcontext* alContext{};
    ALCdevice* alDevice{};
} INX_Audio{};

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

bool INX_AudioState_Init(NX_AppDesc* desc)
{
    INX_Audio.alDevice = alcOpenDevice(nullptr);
    if (INX_Audio.alDevice == nullptr) {
        NX_LogF("AUDIO: Failed to create OpenAL device; ", SDL_GetError());
    }

    INX_Audio.alContext = alcCreateContext(INX_Audio.alDevice, nullptr);
    if (INX_Audio.alContext == nullptr) {
        alcCloseDevice(INX_Audio.alDevice);
        NX_LogF("AUDIO: Failed to create OpenAL context; ", SDL_GetError());
    }

    if (!alcMakeContextCurrent(INX_Audio.alContext)) {
        return false;
    }

    return true;
}

void INX_AudioState_Quit()
{
    alcDestroyContext(INX_Audio.alContext);
    INX_Audio.alContext = nullptr;

    alcCloseDevice(INX_Audio.alDevice);
    INX_Audio.alDevice = nullptr;
}

// ============================================================================
// PUBLIC API
// ============================================================================

float NX_GetAudioVolume()
{
    ALfloat gain = 0;
    alGetListenerf(AL_GAIN, &gain);
    return std::pow(gain, 1.0f / 3.0f);
}

void NX_SetAudioVolume(float volume)
{
    volume = std::max(volume, 0.0f);
    float gain = std::pow(volume, 3.0f);
    alListenerf(AL_GAIN, gain);
}
