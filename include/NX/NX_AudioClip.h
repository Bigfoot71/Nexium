/* NX_AudioClip.h -- API declaration for Nexium's audio clip module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_AUDIO_CLIP_H
#define NX_AUDIO_CLIP_H

#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

typedef struct NX_AudioClip NX_AudioClip;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Load a clip from a file
 * @param filePath Path to the clip file (supports WAV, FLAC, MP3, OGG)
 * @param channelCount Number of channels for polyphony (must be > 0)
 * @return Clip pointer on success, NULL on failure
 */
NXAPI NX_AudioClip* NX_LoadAudioClip(const char* filePath, int channelCount);

/**
 * @brief Destroy a loaded clip and free all associated resources
 * @param clip Clip pointer to destroy
 */
NXAPI void NX_DestroyAudioClip(NX_AudioClip* clip);

/**
 * @brief Play a clip on a specific channel or automatically find a free one
 * @param clip Clip pointer to play
 * @param channel Channel index (>= 0) or negative value to auto-select first available channel
 * @return Channel index where the clip is played, or -1 if no channel is available
 */
NXAPI int NX_PlayAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Pause a clip on a specific channel or all channels
 * @param clip Clip pointer to pause
 * @param channel Channel index (>= 0) to pause specific channel, or negative value to pause all channels
 */
NXAPI void NX_PauseAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Stop a clip on a specific channel or all channels
 * @param clip Clip pointer to stop
 * @param channel Channel index (>= 0) to stop specific channel, or negative value to stop all channels
 */
NXAPI void NX_StopAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Rewind a clip on a specific channel or all channels
 * @param clip Clip pointer to rewind
 * @param channel Channel index (>= 0) to rewind specific channel, or negative value to rewind all channels
 */
NXAPI void NX_RewindAudioClip(NX_AudioClip* clip, int channel);

/**
 * @brief Check if a clip is playing on a specific channel or any channel
 * @param clip Clip pointer to check
 * @param channel Channel index (>= 0) to check specific channel, or negative value to check if playing on any channel
 * @return true if clip is playing on the specified channel(s), false otherwise
 */
NXAPI bool NX_IsAudioClipPlaying(NX_AudioClip* clip, int channel);

/**
 * @brief Get the number of channels assigned to a clip
 * @param clip Clip pointer
 * @return Number of channels, or 0 if clip pointer is invalid
 */
NXAPI int NX_GetAudioClipChannelCount(NX_AudioClip* clip);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_AUDIO_CLIP_H
