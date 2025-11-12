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
    .falloff = 1.0f,                            \
    .cullMask = NX_LAYER_ALL                    \
}

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Defines a probe used for local environment capture or reflection sampling.
 *
 * A probe represents a spatial volume from which lighting or reflections can be 
 * captured or applied. It defines a position, range, and influence falloff, and 
 * specifies which scene layers should be included during environment capture.
 *
 * @note Typically used with NX_IndirectLight for reflection probe rendering.
 * @see NX_DrawReflectionProbe3D
 * @see NX_BeginCubemap3D
 */
typedef struct NX_Probe {
    NX_Vec3 position;   ///< World-space position of the probe center
    float range;        ///< Maximum influence radius in world units
    float falloff;      ///< Smooth attenuation factor near the influence boundary
    NX_Layer cullMask;  ///< Layer mask selecting which objects are captured in the probe
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
