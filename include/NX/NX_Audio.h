/* NX_Audio.h -- API declaration for Nexium's audio module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_AUDIO_H
#define NX_AUDIO_H

#include "./NX_API.h"
#include <stdbool.h>

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Get the master volume
 * @return Master volume (0.0 = mute, 1.0 = max)
 */
NXAPI float NX_GetAudioVolume(void);

/**
 * @brief Set the master volume
 * @param volume Volume value (0.0 = mute, 1.0 = max)
 */
NXAPI void NX_SetAudioVolume(float volume);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_AUDIO_H
