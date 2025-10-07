/* NX_Font.hpp -- Implementation of the API for fonts
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_FONT_HPP
#define NX_RENDER_FONT_HPP

#include <NX/NX_Codepoint.h>
#include <NX/NX_Render.h>
#include <SDL3/SDL_stdinc.h>

#include "../Detail/GPU/Texture.hpp"

/* === Declaration === */

class NX_Font {
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
    NX_Font(const void* fileData, size_t dataSize, NX_FontType type, int baseSize, const int* codepoints, int codepointCount);
    ~NX_Font();

    bool isValid() const;
    int baseSize() const;
    NX_FontType type() const;
    int glyphPadding() const;
    const gpu::Texture& gpuTexture() const;

    const Glyph& getGlyph(int codepoint) const;
    NX_Vec2 measureText(const char* text, float fontSize, NX_Vec2 spacing) const;
    NX_Vec2 measureCodepoints(const int* codepoints, int length, float fontSize, NX_Vec2 spacing) const;

private:
    int getGlyphIndex(int codepoint) const;

private:
    int mBaseSize;          //< Base font size (default character height in pixels)
    int mGlyphCount;        //< Total number of glyphs available in this font
    int mGlyphPadding;      //< Padding around glyphs in the texture atlas
    gpu::Texture mTexture;  //< Texture atlas containing all glyph images
    Glyph* mGlyphs;         //< Array of glyph information structures
    NX_FontType mType;      //< Font rendering type used during text rendering

private:
    static bool generateFontAtlas(NX_Image* atlas, const uint8_t* fileData, int dataSize,
                                  NX_FontType font_type, int baseSize,
                                  const int* codepoints, int codepointCount,
                                  int padding, Glyph** outGlyphs);
};

/* === Public Implementation === */

inline NX_Font::~NX_Font()
{
    SDL_free(mGlyphs);
}

inline bool NX_Font::isValid() const
{
    return mTexture.isValid();
}

inline int NX_Font::baseSize() const
{
    return mBaseSize;
}

inline NX_FontType NX_Font::type() const
{
    return mType;
}

inline int NX_Font::glyphPadding() const
{
    return mGlyphPadding;
}

inline const gpu::Texture& NX_Font::gpuTexture() const
{
    return mTexture;
}

inline const NX_Font::Glyph& NX_Font::getGlyph(int codepoint) const
{
    return mGlyphs[getGlyphIndex(codepoint)];
}

#endif // NX_RENDER_FONT_HPP
