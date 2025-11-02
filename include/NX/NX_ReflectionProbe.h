/* NX_ReflectionProbe.h -- API declarations for Nexium's reflection probe module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_REFLECTION_PROBE_H
#define NX_REFLECTION_PROBE_H

#include "./NX_Cubemap.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Opaque handle to a reflection probe.
 *
 * Represents precomputed environment reflections.
 * Can be used to add realistic reflections on materials.
 */
typedef struct NX_ReflectionProbe NX_ReflectionProbe;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates a reflection probe from a cubemap.
 * @param cubemap Pointer to the cubemap (can be NULL).
 * @return Pointer to a newly created NX_ReflectionProbe.
 * @note Reflection probes capture the environment for specular and diffuse image-based lighting.
 */
NXAPI NX_ReflectionProbe* NX_CreateReflectionProbe(const NX_Cubemap* cubemap);

/**
 * @brief Loads a reflection probe from a cubemap file.
 * @param filePath Path to the cubemap image file.
 * @return Pointer to a newly loaded NX_ReflectionProbe.
 * @note The cubemap is used to generate specular and diffuse reflections.
 */
NXAPI NX_ReflectionProbe* NX_LoadReflectionProbe(const char* filePath);

/**
 * @brief Destroys a reflection probe and frees its resources.
 * @param probe Pointer to the NX_ReflectionProbe to destroy.
 */
NXAPI void NX_DestroyReflectionProbe(NX_ReflectionProbe* probe);

/**
 * @brief Updates an existing reflection probe from a new cubemap.
 * @param probe Pointer to the reflection probe to update.
 * @param cubemap Pointer to the new cubemap used for updating (cannot be NULL).
 */
NXAPI void NX_UpdateReflectionProbe(NX_ReflectionProbe* probe, const NX_Cubemap* cubemap);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_REFLECTION_PROBE_H
