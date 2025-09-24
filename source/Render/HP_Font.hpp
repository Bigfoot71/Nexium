/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef HP_RENDER_FONT_HPP
#define HP_RENDER_FONT_HPP

#include <Hyperion/HP_Codepoint.h>
#include <Hyperion/HP_Render.h>
#include <SDL3/SDL_stdinc.h>

#include "../Detail/GPU/Texture.hpp"

/* === Declaration === */

class HP_Font {
public:
    struct Glyph {
        void* pixels;       //< Pixels of the glyph (R8 unorm)
        int value;          //< Unicode codepoint value
        int xOffset;        //< Horizontal offset when drawing the glyph
        int yOffset;        //< Vertical offset when drawing the glyph
        int xAdvance;       //< Horizontal advance to next character position
        uint16_t xAtlas;    //< X-coordinate position in texture atlas
        uint16_t yAtlas;    //< Y-coordinate position in texture atlas
        uint16_t wGlyph;    //< Width in pixels of the glyph (this also applies to the atlas)
        uint16_t hGlyph;    //< Height in pixels of the glyph (this also applies to the atlas)
    };

public:
    HP_Font(const void* fileData, size_t dataSize, HP_FontType type, int baseSize, const int* codepoints, int codepointCount);
    ~HP_Font();

    bool isValid() const;
    int baseSize() const;
    HP_FontType type() const;
    int glyphPadding() const;
    const gpu::Texture& gpuTexture() const;

    const Glyph& getGlyph(int codepoint) const;
    HP_Vec2 measureText(const char* text, float fontSize, HP_Vec2 spacing) const;
    HP_Vec2 measureCodepoints(const int* codepoints, int length, float fontSize, HP_Vec2 spacing) const;

private:
    int getGlyphIndex(int codepoint) const;

private:
    int mBaseSize;          //< Base font size (default character height in pixels)
    int mGlyphCount;        //< Total number of glyphs available in this font
    int mGlyphPadding;      //< Padding around glyphs in the texture atlas
    gpu::Texture mTexture;  //< Texture atlas containing all glyph images
    Glyph* mGlyphs;         //< Array of glyph information structures
    HP_FontType mType;      //< Font rendering type used during text rendering

private:
    static bool generateFontAtlas(HP_Image* atlas, const uint8_t* fileData, int dataSize,
                                  HP_FontType font_type, int baseSize,
                                  const int* codepoints, int codepointCount,
                                  int padding, Glyph** outGlyphs);
};

/* === Public Implementation === */

inline HP_Font::~HP_Font()
{
    SDL_free(mGlyphs);
}

inline bool HP_Font::isValid() const
{
    return mTexture.isValid();
}

inline int HP_Font::baseSize() const
{
    return mBaseSize;
}

inline HP_FontType HP_Font::type() const
{
    return mType;
}

inline int HP_Font::glyphPadding() const
{
    return mGlyphPadding;
}

inline const gpu::Texture& HP_Font::gpuTexture() const
{
    return mTexture;
}

inline const HP_Font::Glyph& HP_Font::getGlyph(int codepoint) const
{
    return mGlyphs[getGlyphIndex(codepoint)];
}

#endif // HP_RENDER_FONT_HPP
