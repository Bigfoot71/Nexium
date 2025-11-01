/* NX_RenderTexture.h -- API declaration for Nexium's render texture module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_TEXTURE_H
#define NX_RENDER_TEXTURE_H

#include "./NX_Texture.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Opaque handle to a render texture.
 *
 * Represents a render texture containing a depth color target.
 */
typedef struct NX_RenderTexture NX_RenderTexture;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates an off-screen render texture.
 * @param w Width of the render texture in pixels.
 * @param h Height of the render texture in pixels.
 * @return Pointer to the newly created render texture, or NULL on failure.
 */
NXAPI NX_RenderTexture* NX_CreateRenderTexture(int w, int h);

/**
 * @brief Destroys a render texture.
 * @param target Pointer to the render texture to destroy.
 */
NXAPI void NX_DestroyRenderTexture(NX_RenderTexture* target);

/**
 * @brief Retrieves the size of the specified render texture.
 * @param texture Pointer to the NX_RenderTexture to query.
 * @return Render texture size in pixels as an NX_IVec2 (x = width, y = height).
 */
NXAPI NX_IVec2 NX_GetRenderTextureSize(const NX_RenderTexture* target);

/**
 * @brief Retrieves the color texture of a render texture.
 * @param target Pointer to the render texture.
 * @return Pointer to the color texture.
 */
NXAPI NX_Texture* NX_GetRenderTexture(const NX_RenderTexture* target);

/**
 * @brief Blits a render texture to the screen.
 * @param target Pointer to the render texture to draw.
 * @param xDst X coordinate of the destination rectangle (in screen space).
 * @param yDst Y coordinate of the destination rectangle (in screen space).
 * @param wDst Width of the destination rectangle.
 * @param hDst Height of the destination rectangle.
 * @param linear If true, applies linear filtering when scaling; otherwise nearest-neighbor.
 */
NXAPI void NX_BlitRenderTexture(const NX_RenderTexture* target, int xDst, int yDst, int wDst, int hDst, bool linear);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_RENDER_TEXTURE_H
