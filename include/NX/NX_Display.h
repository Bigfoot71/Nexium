/* NX_Display.h -- API declaration for Nexium's display module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_DISPLAY_H
#define NX_DISPLAY_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Gets the display scaling factor.
 * @return Display scale (e.g., 1.0 for standard DPI, >1.0 for HiDPI/Retina).
 */
NXAPI float NX_GetDisplayScale(void);

/**
 * @brief Gets the display DPI (dots per inch).
 */
NXAPI float NX_GetDisplayGetDPI(void);

/**
 * @brief Gets the current display index.
 * @return Display index, useful in multi-monitor setups.
 */
NXAPI int NX_GetDisplayIndex(void);

/**
 * @brief Gets the display size in pixels.
 * @return Integer 2D vector (width, height).
 */
NXAPI NX_IVec2 NX_GetDisplaySize(void);

/**
 * @brief Gets the display size in pixels as floats.
 * @return Float 2D vector (width, height).
 */
NXAPI NX_Vec2 NX_GetDisplaySizeF(void);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_DISPLAY_H
