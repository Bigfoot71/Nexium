/* NX_Runtime.h -- API declaration for Nexium's runtime module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RUNTIME_H
#define NX_RUNTIME_H

#include "./NX_API.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Run one iteration of the main loop.
 * @return false if the program should close.
 * @note Can be used directly in the main loop: `while (NX_FrameStep())`.
 */
NXAPI bool NX_FrameStep(void);

/**
 * @brief Gets the current system time in nanoseconds since Jan 1, 1970 (UTC).
 */
NXAPI int64_t NX_GetCurrentTimeNS(void);

/**
 * @brief Gets the current system time in seconds since Jan 1, 1970 (UTC).
 */
NXAPI double NX_GetCurrentTime(void);

/**
 * @brief Gets the elapsed time since library initialization.
 * @return Elapsed time in seconds.
 */
NXAPI double NX_GetElapsedTime(void);

/**
 * @brief Gets the time taken by the last frame.
 * @return Frame time in seconds.
 */
NXAPI double NX_GetDeltaTime(void);

/**
 * @brief Gets the current frame rate (FPS).
 */
NXAPI int NX_GetFPS(void);

/**
 * @brief Sets the target frame rate (FPS).
 * @param fps Desired target FPS.
 */
NXAPI void NX_SetTargetFPS(int fps);

/**
 * @brief Sets the vertical synchronization mode.
 *
 * VSync synchronizes the frame rate with the display's refresh rate to reduce screen tearing.
 *
 * @param mode The VSync mode:
 *   -  0 : Disabled (unlimited FPS, may cause tearing).
 *   -  1 : Enabled (FPS capped to display refresh rate).
 *   - -1 : Adaptive (enabled only if FPS > refresh rate, if supported).
 *
 * @return true if the VSync mode was successfully applied, false otherwise.
 *
 * @note Adaptive VSync is not supported on all platforms or drivers.
 */
NXAPI bool NX_SetVSync(int mode);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_RUNTIME_H
