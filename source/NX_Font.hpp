/* NX_Font.hpp -- API definitions for Nexium's font module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_FONT_HPP
#define NX_FONT_HPP

#include <NX/NX_Font.h>

#include "./Detail/Util/FixedArray.hpp"
#include "./Detail/Util/Memory.hpp"
#include "./NX_Texture.hpp"

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

struct INX_Glyph {
    util::UniquePtr<uint8_t> pixels;    //< Pixels of the glyph (R8 unorm)
    int value;                          //< Unicode codepoint value
    int xOffset;                        //< Horizontal offset when drawing the glyph
    int yOffset;                        //< Vertical offset when drawing the glyph
    int xAdvance;                       //< Horizontal advance to next character position
    uint16_t xAtlas;                    //< X-coordinate position in texture atlas
    uint16_t yAtlas;                    //< Y-coordinate position in texture atlas
    uint16_t wGlyph;                    //< Width in pixels of the glyph (this also applies to the atlas)
    uint16_t hGlyph;                    //< Height in pixels of the glyph (this also applies to the atlas)
};

struct NX_Font {
    int baseSize{};                       //< Base font size (default character height in pixels)
    int glyphPadding{};                   //< Padding around glyphs in the texture atlas
    NX_Texture* texture{};                //< Texture atlas containing all glyph images
    util::FixedArray<INX_Glyph> glyphs{}; //< Array of glyph information structures
    NX_FontType type{};                   //< Font rendering type used during text rendering
    ~NX_Font();
};

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

const INX_Glyph& INX_GetFontGlyph(const NX_Font* font, int codepoint);

#endif // NX_FONT_HPP
