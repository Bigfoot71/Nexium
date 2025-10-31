/* NX_Audio.c -- API definition for Nexium's audio module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Audio.h>

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>
#include <math.h>
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
// PUBLIC API
// ============================================================================

float NX_GetAudioVolume(void)
{
    ALfloat gain = 0;
    alGetListenerf(AL_GAIN, &gain);
    return powf(gain, 1.0f / 3.0f);
}

void NX_SetAudioVolume(float volume)
{
    volume = fmaxf(volume, 0.0f);
    float gain = powf(volume, 3.0f);
    alListenerf(AL_GAIN, gain);
}
