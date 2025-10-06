/* HP_Image.h -- API declaration for Hyperion's image module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_IMAGE_H
#define HP_IMAGE_H

#include "./HP_Math.h"

/* === Enums === */

/**
 * @brief Pixel format enumeration
 */
typedef enum HP_PixelFormat {
    HP_PIXEL_FORMAT_R8,         ///< Single channel 8-bit red
    HP_PIXEL_FORMAT_RG8,        ///< Dual channel 8-bit red-green
    HP_PIXEL_FORMAT_RGB8,       ///< Three channel 8-bit red-green-blue
    HP_PIXEL_FORMAT_RGBA8,      ///< Four channel 8-bit red-green-blue-alpha
    HP_PIXEL_FORMAT_R16F,       ///< Single channel 16-bit float red
    HP_PIXEL_FORMAT_RG16F,      ///< Dual channel 16-bit float red-green
    HP_PIXEL_FORMAT_RGB16F,     ///< Three channel 16-bit float red-green-blue
    HP_PIXEL_FORMAT_RGBA16F,    ///< Four channel 16-bit float red-green-blue-alpha
    HP_PIXEL_FORMAT_R32F,       ///< Single channel 32-bit float red
    HP_PIXEL_FORMAT_RG32F,      ///< Dual channel 32-bit float red-green
    HP_PIXEL_FORMAT_RGB32F,     ///< Three channel 32-bit float red-green-blue
    HP_PIXEL_FORMAT_RGBA32F,    ///< Four channel 32-bit float red-green-blue-alpha
} HP_PixelFormat;

/* === Structures === */

/**
 * @brief Image data structure
 */
typedef struct HP_Image {
    void* pixels;               ///< Pointer to pixel data buffer
    int w, h;                   ///< Image width and height in pixels
    HP_PixelFormat format;      ///< Pixel format of the image data
} HP_Image;

/* === Public API === */

#if defined(__cplusplus)
extern "C" {
#endif

/** @defgroup Pixel Pixel Functions
 *  @brief Low-level pixel manipulation functions
 *  @{
 */

/**
 * @brief Get the number of bytes per pixel for a given format
 * @param format The pixel format
 * @return Number of bytes per pixel
 */
HPAPI int HP_GetPixelBytes(HP_PixelFormat format);

/**
 * @brief Get the number of channels for a given pixel format
 * @param format The pixel format
 * @return Number of channels (1-4)
 */
HPAPI int HP_GetPixelChannels(HP_PixelFormat format);

/**
 * @brief Get the number of bytes per channel for a given format
 * @param format The pixel format
 * @return Number of bytes per channel
 */
HPAPI int HP_GetPixelChannelBytes(HP_PixelFormat format);

/**
 * @brief Write a color value to a specific pixel in a pixel buffer
 * @param pixels Pointer to the pixel buffer
 * @param index Pixel index in the buffer
 * @param format Pixel format of the buffer
 * @param color Color value to write
 */
HPAPI void HP_WritePixel(void* pixels, int index, HP_PixelFormat format, HP_Color color);

/**
 * @brief Read a color value from a specific pixel in a pixel buffer
 * @param pixels Pointer to the pixel buffer
 * @param index Pixel index in the buffer
 * @param format Pixel format of the buffer
 * @return Color value read from the pixel
 */
HPAPI HP_Color HP_ReadPixel(const void* pixels, int index, HP_PixelFormat format);

/** @} */ // end of Pixel

/** @defgroup Image Image Functions
 *  @brief High-level image creation, manipulation and processing functions
 *  @{
 */

/**
 * @brief Create a new image filled with zeros
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param format Pixel format for the new image
 * @return New image structure with allocated pixel buffer
 */
HPAPI HP_Image HP_CreateImage(int w, int h, HP_PixelFormat format);

/**
 * @brief Create an image from raw pixel data with optional format conversion
 *
 * Creates a new image by copying raw pixel data. If source and destination
 * formats are identical, performs a simple memcpy. Otherwise, converts
 * between formats during the copy operation.
 *
 * @param pixels Source pixel data buffer
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param srcFormat Source pixel format
 * @param dstFormat Destination pixel format
 * @return New image with converted pixel data
 */
HPAPI HP_Image HP_CreateImageFromMem(const void* pixels, int w, int h, HP_PixelFormat srcFormat, HP_PixelFormat dstFormat);

/**
 * @brief Load and decode an image from file
 *
 * Supports the following formats: PNG, TGA, HDR, JPG, BMP, PIC, PNM.
 * Automatically converts L/LA images to RGB/RGBA and selects the appropriate pixel format.
 *
 * @param filePath Path to the image file
 * @return Decoded image, ready for use in rendering, or empty image on failure
 */
HPAPI HP_Image HP_LoadImage(const char* filePath);

/**
 * @brief Load raw image data from file without any channel conversion
 *
 * Supports the following formats: PNG, TGA, HDR, JPG, BMP, PIC, PNM.
 * Preserves the original number of channels and pixel layout as stored in the file.
 *
 * @param filePath Path to the image file
 * @return Image containing raw pixel data as-is, or empty image on failure
 */
HPAPI HP_Image HP_LoadImageAsData(const char* filePath);

/**
 * @brief Decode an image from memory buffer
 *
 * Supports the following formats: PNG, TGA, HDR, JPG, BMP, PIC, PNM.
 * Automatically converts L/LA images to RGB/RGBA and selects the appropriate pixel format.
 *
 * @param data Pointer to encoded image data in memory
 * @param size Size of the encoded data in bytes
 * @return Decoded image, ready for use in rendering, or empty image on failure
 */
HPAPI HP_Image HP_LoadImageFromMem(const void* data, size_t size);

/**
 * @brief Load raw image data from memory buffer without any channel conversion
 *
 * Supports the following formats: PNG, TGA, HDR, JPG, BMP, PIC, PNM.
 * Preserves the original number of channels and pixel layout as stored in the memory buffer.
 *
 * @param data Pointer to encoded image data in memory
 * @param size Size of the encoded data in bytes
 * @return Image containing raw pixel data as-is, or empty image on failure
 */
HPAPI HP_Image HP_LoadImageAsDataFromMem(const void* data, size_t size);

/**
 * @brief Destroy an image and free its resources
 * @param image Pointer to the image to destroy
 * @note Sets the image structure to zero after freeing resources
 */
HPAPI void HP_DestroyImage(HP_Image* image);

/**
 * @brief Generate a solid color image
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param color Fill color
 * @return Generated image filled with the specified color
 * @note Pixel format is automatically selected based on color characteristics (8-bit, 16-bit float, or 32-bit float, with/without alpha)
 */
HPAPI HP_Image HP_GenImageColor(int w, int h, HP_Color color);

/**
 * @brief Generate a linear gradient image
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param direction Gradient direction (0=horizontal, 1=vertical)
 * @param start Starting color of the gradient
 * @param end Ending color of the gradient
 * @return Generated linear gradient image
 * @note Pixel format is automatically selected based on color characteristics (8-bit, 16-bit float, or 32-bit float, with/without alpha)
 */
HPAPI HP_Image HP_GenImageGradientLinear(int w, int h, int direction, HP_Color start, HP_Color end);

/**
 * @brief Generate a radial gradient image
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param density Gradient density factor
 * @param inner Inner (center) color
 * @param outer Outer (edge) color
 * @return Generated radial gradient image
 * @note Pixel format is automatically selected based on color characteristics (8-bit, 16-bit float, or 32-bit float, with/without alpha)
 */
HPAPI HP_Image HP_GenImageGradientRadial(int w, int h, float density, HP_Color inner, HP_Color outer);

/**
 * @brief Generate a square gradient image
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param density Gradient density factor
 * @param inner Inner (center) color
 * @param outer Outer (edge) color
 * @return Generated square gradient image
 * @note Pixel format is automatically selected based on color characteristics (8-bit, 16-bit float, or 32-bit float, with/without alpha)
 */
HPAPI HP_Image HP_GenImageGradientSquare(int w, int h, float density, HP_Color inner, HP_Color outer);

/**
 * @brief Generate a checkerboard pattern image
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param xChecks Number of checks horizontally
 * @param yChecks Number of checks vertically
 * @param c0 First checker color
 * @param c1 Second checker color
 * @return Generated checkerboard image
 * @note Pixel format is automatically selected based on color characteristics (8-bit, 16-bit float, or 32-bit float, with/without alpha)
 */
HPAPI HP_Image HP_GenImageChecked(int w, int h, int xChecks, int yChecks, HP_Color c0, HP_Color c1);

/**
 * @brief Composes an RGB image by mapping each source image to its corresponding color channel.
 *
 * This function creates a new RGB image by sampling the red, green, and blue channels
 * from up to three source images and placing them in the corresponding channels of
 * the output image:
 *   - R channel from sources[0]
 *   - G channel from sources[1]
 *   - B channel from sources[2]
 *
 * At least one source must be non-NULL to produce a valid output image.
 *
 * Source images can have different dimensions; the output image will have a width and height
 * equal to the maximum width and height among all non-NULL sources. Nearest neighbor
 * sampling is used for rescaling each channel, using fixed-point 16.16 arithmetic.
 *
 * @param sources Array of 3 source image pointers (can be NULL). Each index maps to the
 *                corresponding output channel (0 = red, 1 = green, 2 = blue).
 * @param defaultColor Default color used for channels when the corresponding source is NULL.
 *
 * @return New composed RGB image, or an empty image if the composition fails.
 *
 * @note This function is particularly useful for composing ORM (Occlusion/Roughness/Metalness)
 *       textures from separate grayscale sources.
 *
 * @example
 * ```c
 * const HP_Image* sources[3] = {occlusionMap, roughnessMap, metalnessMap};
 * HP_Image ormImage = HP_ComposeImagesRGB(sources, HP_WHITE);
 * ```
 */
HPAPI HP_Image HP_ComposeImagesRGB(const HP_Image* sources[3], HP_Color defaultColor);

/**
 * @brief Set a pixel color at specific coordinates with bounds checking
 *
 * Performs boundary and validity checks before setting the pixel.
 * Use HP_WritePixel for better performance if bounds checking is not needed.
 *
 * @param image Target image
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color to set
 */
HPAPI void HP_SetImagePixel(const HP_Image* image, int x, int y, HP_Color color);

/**
 * @brief Get a pixel color at specific coordinates with bounds checking
 *
 * Performs boundary and validity checks before reading the pixel.
 * Use HP_ReadPixel for better performance if bounds checking is not needed.
 *
 * @param image Source image
 * @param x X coordinate
 * @param y Y coordinate
 * @return Color value at the specified coordinates, or default color if out of bounds
 */
HPAPI HP_Color HP_GetImagePixel(const HP_Image* image, int x, int y);

/**
 * @brief Convert an image to a different pixel format
 * @param image Image to convert (modified in place)
 * @param format Target pixel format
 */
HPAPI void HP_ConvertImage(HP_Image* image, HP_PixelFormat format);

/**
 * @brief Invert the colors of an image
 *
 * Inverts all color channels using the formula: result = 1.0 - color
 * Alpha channel is preserved if present.
 *
 * @param image Image to invert (modified in place)
 */
HPAPI void HP_InvertImage(const HP_Image* image);

/**
 * @brief Blits a rectangular region from source image to destination using nearest neighbor sampling.
 *
 * Copies and scales a rectangular region from the source image to a rectangular region in the
 * destination image using nearest neighbor sampling with fixed-point 16.16 arithmetic.
 * Regions are automatically clamped to image boundaries.
 *
 * @param src Source image to copy from
 * @param srcX Source rectangle X coordinate
 * @param srcY Source rectangle Y coordinate
 * @param srcW Source rectangle width
 * @param srcH Source rectangle height
 * @param dst Destination image to copy to
 * @param dstX Destination rectangle X coordinate
 * @param dstY Destination rectangle Y coordinate
 * @param dstW Destination rectangle width
 * @param dstH Destination rectangle height
 */
HPAPI void HP_BlitImage(
    const HP_Image* src, int srcX, int srcY, int srcW, int srcH,
    const HP_Image* dst, int dstX, int dstY, int dstW, int dstH);

/** @} */ // end of Image

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // HP_IMAGE_H
