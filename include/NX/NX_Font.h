/* NX_Font.h -- API declaration for Nexium's font module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_FONT_H
#define NX_FONT_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Defines the type of font used for rendering.
 */
typedef enum NX_FontType {
    NX_FONT_NORMAL,                 ///< Standard vector font, anti-aliased, general-purpose text.
    NX_FONT_LIGHT,                  ///< Light/thin vector font, finer strokes, good for small UI text.
    NX_FONT_MONO,                   ///< Monochrome bitmap font, pixel-perfect, very fast to load.
    NX_FONT_SDF                     ///< Signed Distance Field font, scalable, smooth rendering at arbitrary sizes.
} NX_FontType;

/**
 * @brief Opaque handle to a font stored on the GPU.
 *
 * Represents a loaded font for text rendering.
 * Supports bitmap or SDF rendering modes depending on NX_FontType.
 */
typedef struct NX_Font NX_Font;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Loads a font from a file.
 * @param filePath Path to the font file.
 * @param type Font type (bitmap or SDF).
 * @param baseSize Base size of the font in pixels.
 * @param codepoints Array of Unicode codepoints to load (can be NULL to load default set).
 * @param codepointCount Number of codepoints in the array.
 * @return Pointer to a newly loaded NX_Font.
 */
NXAPI NX_Font* NX_LoadFont(const char* filePath, NX_FontType type, int baseSize, const int* codepoints, int codepointCount);

/**
 * @brief Loads a font from memory.
 * @param fileData Pointer to font file data in memory.
 * @param dataSize Size of the font data in bytes.
 * @param type Font type (bitmap or SDF).
 * @param baseSize Base size of the font in pixels.
 * @param codepoints Array of Unicode codepoints to load (can be NULL to load default set).
 * @param codepointCount Number of codepoints in the array.
 * @return Pointer to a newly loaded NX_Font.
 */
NXAPI NX_Font* NX_LoadFontFromData(const void* fileData, size_t dataSize, NX_FontType type, int baseSize, const int* codepoints, int codepointCount);

/**
 * @brief Destroys a font and frees its resources.
 * @param font Pointer to the NX_Font to destroy.
 */
NXAPI void NX_DestroyFont(NX_Font* font);

/**
 * @brief Retrieves the font type of the specified font.
 * @param font Pointer to the NX_Font to query.
 * @return The font type.
 */
NXAPI NX_FontType NX_GetFontType(const NX_Font* font);

/**
 * @brief Measures the size of an array of codepoints in the given font.
 * @param font Pointer to the NX_Font to use (can be NULL to use the default font).
 * @param codepoints Array of Unicode codepoints to measure.
 * @param length Number of codepoints in the array.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 * @return Width and height required to render the codepoints.
 */
NXAPI NX_Vec2 NX_MeasureCodepoints(const NX_Font* font, const int* codepoints, int length, float fontSize, NX_Vec2 spacing);

/**
 * @brief Measures the size of a text string in the given font.
 * @param font Pointer to the NX_Font to use (can be NULL to use the default font).
 * @param text Null-terminated string to measure.
 * @param fontSize Font size in pixels.
 * @param spacing Additional spacing between characters.
 * @return Width and height required to render the text.
 */
NXAPI NX_Vec2 NX_MeasureText(const NX_Font* font, const char* text, float fontSize, NX_Vec2 spacing);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_FONT_H
