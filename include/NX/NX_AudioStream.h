/* NX_AudioStream.h -- API declaration for Nexium's audio stream module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_AUDIO_STREAM_H
#define NX_AUDIO_STREAM_H

#include "./NX_API.h"
#include <stdbool.h>

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

typedef struct NX_AudioStream NX_AudioStream;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Load stream from a file
 * @param filePath Path to the stream file
 * @return Stream pointer on success, NULL on failure
 */
NXAPI NX_AudioStream* NX_LoadAudioStream(const char* filePath);

/**
 * @brief Destroy loaded stream
 * @param stream Stream pointer
 */
NXAPI void NX_DestroyAudioStream(NX_AudioStream* stream);

/**
 * @brief Play stream
 * @param stream Stream pointer
 */
NXAPI void NX_PlayAudioStream(NX_AudioStream* stream);

/**
 * @brief Pause stream
 * @param stream Stream pointer
 */
NXAPI void NX_PauseAudioStream(NX_AudioStream* stream);

/**
 * @brief Stop stream
 * @param stream Stream pointer
 */
NXAPI void NX_StopAudioStream(NX_AudioStream* stream);

/**
 * @brief Rewind stream to the beginning
 * @param stream Stream pointer
 */
NXAPI void NX_RewindAudioStream(NX_AudioStream* stream);

/**
 * @brief Check if stream is currently playing
 * @param stream Stream pointer
 * @return true if playing, false otherwise
 */
NXAPI bool NX_IsAudioStreamPlaying(NX_AudioStream* stream);

/**
 * @brief Get current looping state of stream
 * @param stream Stream pointer
 * @return true if looping is enabled, false otherwise
 */
NXAPI bool NX_GetAudioStreamLoop(NX_AudioStream* stream);

/**
 * @brief Enable or disable looping for stream
 * @param stream Stream pointer
 * @param loop true to loop continuously, false to stop at the end
 */
NXAPI void NX_SetAudioStreamLoop(NX_AudioStream* stream, bool loop);

/**
 * @brief Get the total duration of the audio stream in seconds.
 * @param stream Pointer to the audio stream.
 * @return Duration of the stream in seconds.
 */
NXAPI float NX_GetAudioStreamDuration(const NX_AudioStream* stream);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_AUDIO_STREAM_H
