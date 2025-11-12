/* NX_IndirectLight.h -- API declarations for Nexium's indirect light module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_INDIRECT_LIGHT_H
#define NX_INDIRECT_LIGHT_H

#include "./NX_Cubemap.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Opaque handle to a indirect light.
 *
 * Represents precomputed environment reflections.
 * Can be used to add realistic reflections on materials.
 */
typedef struct NX_IndirectLight NX_IndirectLight;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates a indirect light from a cubemap.
 * @param cubemap Pointer to the cubemap (can be NULL).
 * @return Pointer to a newly created NX_IndirectLight.
 */
NXAPI NX_IndirectLight* NX_CreateIndirectLight(const NX_Cubemap* cubemap);

/**
 * @brief Loads a indirect light from a cubemap file.
 * @param filePath Path to the cubemap image file.
 * @return Pointer to a newly loaded NX_IndirectLight.
 * @note The cubemap is used to generate specular and diffuse reflections.
 */
NXAPI NX_IndirectLight* NX_LoadIndirectLight(const char* filePath);

/**
 * @brief Destroys a indirect light and frees its resources.
 * @param indirectLight Pointer to the NX_IndirectLight to destroy.
 */
NXAPI void NX_DestroyIndirectLight(NX_IndirectLight* indirectLight);

/**
 * @brief Updates an existing indirect light from a new cubemap.
 * @param indirectLight Pointer to the indirect light to update.
 * @param cubemap Pointer to the new cubemap used for updating (cannot be NULL).
 */
NXAPI void NX_UpdateIndirectLight(NX_IndirectLight* indirectLight, const NX_Cubemap* cubemap);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_INDIRECT_LIGHT_H
