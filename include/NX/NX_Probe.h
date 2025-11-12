/* NX_Probe.h -- API declaration for Nexium's probe module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_PROBE_H
#define NX_PROBE_H

#include "./NX_Camera.h"
#include "./NX_API.h"

// ============================================================================
// MACROS DEFINITIONS
// ============================================================================

#define NX_BASE_PROBE NX_LITERAL(NX_Probe)      \
{                                               \
    .position = NX_VEC3_ZERO,                   \
    .range = 16.0f,                             \
    .cullMask = NX_LAYER_ALL                    \
}

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

typedef struct NX_Probe {
    NX_Vec3 position;
    float range;
    NX_Layer cullMask;
} NX_Probe;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Returns the current default camera.
 *
 * If no camera was set, returns NX_BASE_CAMERA by default.
 */
NXAPI NX_Probe NX_GetDefaultProbe(void);

/**
 * @brief Sets the default camera used by Nexium.
 *
 * Overrides the camera returned by NX_GetDefaultCamera().
 * Pass NULL to restore the default NX_BASE_CAMERA.
 */
NXAPI void NX_SetDefaultProbe(const NX_Probe* probe);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_PROBE_H
