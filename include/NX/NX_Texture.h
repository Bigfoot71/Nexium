/* NX_Texture.h -- API declaration for Nexium's texture module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_TEXTURE_H
#define NX_TEXTURE_H

#include "./NX_Image.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Defines the texture filtering method.
 *
 * Determines how textures are sampled when scaled.
 */
typedef enum NX_TextureFilter {
    NX_TEXTURE_FILTER_POINT,        ///< Nearest-neighbor filtering. Fastest, pixelated look.
    NX_TEXTURE_FILTER_BILINEAR,     ///< Linear interpolation between 4 texels. Smooth but slightly blurry.
    NX_TEXTURE_FILTER_TRILINEAR     ///< Linear interpolation with mipmaps. Smooth and reduces aliasing at distance.
} NX_TextureFilter;

/**
 * @brief Defines the texture wrapping mode.
 *
 * Determines behavior when texture coordinates exceed [0, 1].
 */
typedef enum NX_TextureWrap {
    NX_TEXTURE_WRAP_CLAMP,          ///< Coordinates outside [0,1] are clamped to the edge pixel.
    NX_TEXTURE_WRAP_REPEAT,         ///< Texture repeats/tiled across the surface.
    NX_TEXTURE_WRAP_MIRROR          ///< Texture repeats but mirrors on each tile.
} NX_TextureWrap;

/**
 * @brief Opaque handle to a GPU texture.
 *
 * Represents a 2D image stored on the GPU.
 * Can be used for material maps or UI elements.
 */
typedef struct NX_Texture NX_Texture;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Gets the current default texture filter used for newly created textures.
 * @return The current default texture filtering mode (NX_TEXTURE_FILTER_BILINEAR by default).
 */
NXAPI NX_TextureFilter NX_GetDefaultTextureFilter(void);

/**
 * @brief Sets the default texture filter for newly created textures.
 * @param filter Default texture filtering mode.
 * @note The default filter is NX_TEXTURE_FILTER_BILINEAR.
 * @note If NX_TEXTURE_FILTER_TRILINEAR is set, mipmaps will be generated automatically for all new textures.
 */
NXAPI void NX_SetDefaultTextureFilter(NX_TextureFilter filter);

/**
 * @brief Gets the current default anisotropy level used for newly created textures.
 * @return The current default anisotropy level (1.0f by default).
 */
NXAPI float NX_GetDefaultTextureAnisotropy(void);

/**
 * @brief Sets the default anisotropy level for newly created textures.
 * @param anisotropy Default anisotropy level (1.0f by default).
 * @note Anisotropy may have no effect on GLES 3.2 depending on platform support.
 * @note The value is automatically clamped to the maximum supported by the platform.
 */
NXAPI void NX_SetDefaultTextureAnisotropy(float anisotropy);

/**
 * @brief Create a texture with the specified dimensions, format, and optional pixel data
 * @param w Texture width in pixels
 * @param h Texture height in pixels
 * @param data Pointer to raw pixel data matching the specified format (can be NULL to create an empty texture)
 * @param format Texture pixel format
 * @return Newly created texture
 */
NXAPI NX_Texture* NX_CreateTexture(int w, int h, const void* data, NX_PixelFormat format);

/**
 * @brief Create a texture with the specified dimensions, format, and optional pixel data, with extended parameters
 * @param w Texture width in pixels
 * @param h Texture height in pixels
 * @param data Pointer to raw pixel data matching the specified format (can be NULL to create an empty texture)
 * @param format Texture pixel format
 * @param wrap Texture wrap mode (defines how texture coordinates outside [0,1] are handled)
 * @param filter Texture filtering mode (defines how sampling between texels is performed)
 * @return Newly created texture with the specified wrap and filter settings
 */
NXAPI NX_Texture* NX_CreateTextureEx(int w, int h, const void* data, NX_PixelFormat format,
                                     NX_TextureWrap wrap, NX_TextureFilter filter);

/**
 * @brief Creates a GPU texture from an image.
 * @param image Pointer to the source image.
 * @return Pointer to a newly created NX_Texture, or NULL on failure.
 */
NXAPI NX_Texture* NX_CreateTextureFromImage(const NX_Image* image);

/**
 * @brief Creates a GPU texture from an image, with extended wrap and filter options
 * @param image Pointer to the source image
 * @param wrap Texture wrap mode (defines how texture coordinates outside [0,1] are handled)
 * @param filter Texture filtering mode (defines how sampling between texels is performed)
 * @return Pointer to a newly created NX_Texture, or NULL on failure
 */
NXAPI NX_Texture* NX_CreateTextureFromImageEx(const NX_Image* image, NX_TextureWrap wrap, NX_TextureFilter filter);

/**
 * @brief Load a texture from a file and decode it for rendering.
 *
 * Automatically converts pixel formats if needed (e.g., L/LA -> RGB/RGBA)
 *
 * @param filePath Path to the texture file
 * @return Pointer to a newly loaded NX_Texture ready for rendering, or NULL on failure
 */
NXAPI NX_Texture* NX_LoadTexture(const char* filePath);

/**
 * @brief Load raw texture data from a file without decoding or conversion.
 *
 * Preserves the original number of channels and pixel layout as stored in the file.
 * Useful if you want to handle texture decoding manually or keep raw data.
 *
 * @param filePath Path to the texture file
 * @return Pointer to a newly loaded NX_Texture containing raw pixel data, or NULL on failure
 */
NXAPI NX_Texture* NX_LoadTextureAsData(const char* filePath);

/**
 * @brief Destroys a GPU texture and frees its resources.
 * @param texture Pointer to the NX_Texture to destroy.
 */
NXAPI void NX_DestroyTexture(NX_Texture* texture);

/**
 * @brief Retrieves the size of the specified texture.
 * @param texture Pointer to the NX_Texture to query.
 * @return Texture size in pixels as an NX_IVec2 (x = width, y = height).
 */
NXAPI NX_IVec2 NX_GetTextureSize(const NX_Texture* texture);

/**
 * @brief Sets filtering, wrapping, and anisotropy parameters for a texture.
 * @param texture Pointer to the NX_Texture.
 * @param filter Texture filtering mode.
 * @param wrap Texture wrapping mode.
 * @param anisotropy Anisotropy level.
 * @note Anisotropy may have no effect on GLES 3.2 depending on platform support.
 * @note The value is automatically clamped to the maximum supported by the platform.
 */
NXAPI void NX_SetTextureParameters(NX_Texture* texture, NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy);

/**
 * @brief Sets the texture filtering mode.
 * @param texture Pointer to the NX_Texture.
 * @param filter Texture filtering mode.
 */
NXAPI void NX_SetTextureFilter(NX_Texture* texture, NX_TextureFilter filter);

/**
 * @brief Sets the anisotropy level for a texture.
 * @param texture Pointer to the NX_Texture.
 * @param anisotropy Anisotropy level.
 * @note Anisotropy may have no effect on GLES 3.2 depending on platform support.
 * @note The value is automatically clamped to the maximum supported by the platform.
 */
NXAPI void NX_SetTextureAnisotropy(NX_Texture* texture, float anisotropy);

/**
 * @brief Sets the texture wrapping mode.
 * @param texture Pointer to the NX_Texture.
 * @param wrap Texture wrapping mode.
 */
NXAPI void NX_SetTextureWrap(NX_Texture* texture, NX_TextureWrap wrap);

/**
 * @brief Upload the given image to a texture of matching size
 * @param texture Destination texture
 * @param image Source image to upload
 * @note If the image format differs from the texture's internal format, a temporary converted copy is created, which may be performance-costly.
 *       Existing mipmaps in the texture are automatically regenerated after upload.
 */
NXAPI void NX_UploadTexture(NX_Texture* texture, const NX_Image* image);

/**
 * @brief Generates mipmaps for a texture.
 * @param texture Pointer to the NX_Texture.
 */
NXAPI void NX_GenerateMipmap(NX_Texture* texture);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_TEXTURE_H
