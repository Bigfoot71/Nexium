/* NX_Cubemap.h -- API declarations for Nexium's cubemap module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CUBEMAP_H
#define NX_CUBEMAP_H

#include "./NX_Image.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Describes parameters for procedural skybox generation.
 *
 * This structure defines the appearance of a procedural skybox,
 * including sun orientation, sky gradients, ground color, and
 * atmospheric effects such as haze.
 */
typedef struct NX_Skybox {
    NX_Vec3 sunDirection;     ///< Direction of the sun (world space).
    NX_Color skyColorTop;     ///< Sky color at the zenith (top).
    NX_Color skyColorHorizon; ///< Sky color at the horizon.
    NX_Color sunColor;        ///< Color of the sun disk and light.
    NX_Color groundColor;     ///< Ground or floor color.
    float sunSize;            ///< Apparent angular size of the sun (in radians).
    float haze;               ///< Strength of atmospheric haze/scattering (0 = none).
    float energy;             ///< Intensity/brightness multiplier for the sky lighting.
} NX_Skybox;

/**
 * @brief Opaque handle to a cubemap texture.
 *
 * Cubemaps are used for skyboxes or for generating reflection probes.
 * Stores 6 textures corresponding to the faces of a cube.
 */
typedef struct NX_Cubemap NX_Cubemap;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates an empty cubemap.
 *
 * Allocates a cubemap texture ready to be filled,
 * either by procedural skybox or rendering a scene.
 *
 * @param size Edge size (in pixels) of each face.
 * @param format Pixel format for the cubemap faces.
 * @return Pointer to a newly created NX_Cubemap.
 * @note On OpenGL ES, requested 32-bit formats may be downgraded to 16-bit depending on hardware support.
 */
NXAPI NX_Cubemap* NX_CreateCubemap(int size, NX_PixelFormat format);

/**
 * @brief Load a cubemap from an image.
 * @param image Pointer to the source NX_Image.
 * @return Pointer to a newly created NX_Cubemap.
 * @note Cubemaps are used for skyboxes or to generate reflection probes.
 * @note Supported image layouts (auto-detected):
 *   - Equirectangular (panorama)
 *   - Horizontal line (faces packed in OpenGL order)
 *   - Vertical line (faces packed in OpenGL order)
 *   - 4x3 cross:
 *           [+Y]
 *       [-X][+Z][+X][-Z]
 *           [-Y]
 *   - 3x4 cross:
 *           [+Y]
 *       [-X][+Z][+X]
 *           [-Y]
 *           [-Z]
 */
NXAPI NX_Cubemap* NX_LoadCubemapFromData(const NX_Image* image);

/**
 * @brief Loads a cubemap from a file.
 * @param filePath Path to the image file.
 * @return Pointer to a newly loaded NX_Cubemap.
 * @note Cubemaps are used for skyboxes or to generate reflection probes.
 * @note Supported image layouts (auto-detected, same as NX_CreateCubemap).
 */
NXAPI NX_Cubemap* NX_LoadCubemap(const char* filePath);

/**
 * @brief Destroys a cubemap and frees its resources.
 * @param cubemap Pointer to the NX_Cubemap to destroy.
 */
NXAPI void NX_DestroyCubemap(NX_Cubemap* cubemap);

/**
 * @brief Generates a procedural skybox into a cubemap.
 * @param cubemap Destination cubemap to render into.
 * @param skybox Pointer to a skybox description (NX_Skybox).
 */
NXAPI void NX_GenerateSkybox(NX_Cubemap* cubemap, const NX_Skybox* skybox);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_CUBEMAP_H
