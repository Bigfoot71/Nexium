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

#include "./HP_Font.hpp"

#include "../Core/HP_InternalLog.hpp"

/* === FreeType Includes === */

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

/* === STB Rect Pack Implementation === */

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

/* === Public Implementation === */

HP_Font::HP_Font(const void* fileData, size_t dataSize, HP_FontType type, int baseSize, const int* codepoints, int codepointCount)
{
#   define FONT_TTF_DEFAULT_SIZE           32
#   define FONT_TTF_DEFAULT_NUMCHARS       95
#   define FONT_TTF_DEFAULT_FIRST_CHAR     32
#   define FONT_TTF_DEFAULT_CHARS_PADDING   4

    /* --- Base configuration --- */

    codepointCount = (codepointCount > 0) ? codepointCount : FONT_TTF_DEFAULT_NUMCHARS;
    mGlyphPadding = FONT_TTF_DEFAULT_CHARS_PADDING;
    mGlyphCount = codepointCount;
    mBaseSize = baseSize;
    mType = type;

    /* --- Generation of the atlas image --- */

    HP_Image atlas{};

    bool atlasGenerated = generateFontAtlas(
        &atlas,
        static_cast<const uint8_t*>(fileData),
        dataSize,
        type,
        baseSize,
        codepoints,
        codepointCount,
        mGlyphPadding,
        &mGlyphs
    );

    if (!atlasGenerated) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to generate font atlas");
        SDL_free(mGlyphs);
        return;
    }

    /* --- Creating the atlas texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_R8,
            .data = atlas.pixels,
            .width = atlas.w,
            .height = atlas.h
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );
    HP_DestroyImage(&atlas);

    if (!mTexture.isValid()) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to upload font atlas");
        SDL_free(mGlyphs);
        return;
    }
}

HP_Vec2 HP_Font::measureText(const char* text, float fontSize, HP_Vec2 spacing) const
{
    float scale = fontSize / mBaseSize;

    float maxWidth = 0.0f;
    float currentWidth = 0.0f;
    float textHeight = fontSize;

    int maxCharsInLine = 0;
    int currentCharsInLine = 0;
    int textLength = (int)strlen(text);

    for (int i = 0; i < textLength;)
    {
        int codepoint_byte_count = 0;
        int letter = HP_GetCodepointNext(&text[i], &codepoint_byte_count);
        i += codepoint_byte_count;

        if (letter == '\n') {
            maxWidth = fmaxf(maxWidth, currentWidth);
            maxCharsInLine = HP_MAX(maxCharsInLine, currentCharsInLine);
            currentWidth = 0.0f;
            currentCharsInLine = 0;
            textHeight += fontSize + spacing.y;
        }
        else {
            int index = getGlyphIndex(letter);
            float char_width = (mGlyphs[index].xAdvance > 0)
                ? mGlyphs[index].xAdvance
                : (mGlyphs[index].wGlyph + mGlyphs[index].xOffset);

            currentWidth += char_width;
            currentCharsInLine++;
        }
    }

    // Treat the last line
    maxWidth = fmaxf(maxWidth, currentWidth);
    maxCharsInLine = HP_MAX(maxCharsInLine, currentCharsInLine);

    return HP_Vec2 {
        .x = maxWidth * scale + (maxCharsInLine > 0 ? (maxCharsInLine - 1) * spacing.x : 0),
        .y = textHeight
    };
}

HP_Vec2 HP_Font::measureCodepoints(const int* codepoints, int length, float fontSize, HP_Vec2 spacing) const
{
    float scale = fontSize / mBaseSize;

    float maxWidth = 0.0f;
    float currentWidth = 0.0f;
    float textHeight = fontSize;

    int maxCharsInLine = 0;
    int currentCharsInLine = 0;

    for (int i = 0; i < length; i++)
    {
        int letter = codepoints[i];

        if (letter == '\n') {
            maxWidth = fmaxf(maxWidth, currentWidth);
            maxCharsInLine = HP_MAX(maxCharsInLine, currentCharsInLine);
            currentWidth = 0.0f;
            currentCharsInLine = 0;
            textHeight += fontSize + spacing.y;
        }
        else {
            int index = getGlyphIndex(letter);
            float char_width = (mGlyphs[index].xAdvance > 0)
                ? mGlyphs[index].xAdvance
                : (mGlyphs[index].wGlyph + mGlyphs[index].xOffset);

            currentWidth += char_width;
            currentCharsInLine++;
        }
    }

    // Treat the last line
    maxWidth = fmaxf(maxWidth, currentWidth);
    maxCharsInLine = HP_MAX(maxCharsInLine, currentCharsInLine);

    return HP_Vec2 {
        .x = maxWidth * scale + (maxCharsInLine > 0 ? (maxCharsInLine - 1) * spacing.x : 0),
        .y = textHeight
    };
}

/* === Private Implementation === */

int HP_Font::getGlyphIndex(int codepoint) const
{
#   define FALLBACK 63 //< Fallback is '?'

    int index = 0, fallbackIndex = 0;

    for (int i = 0; i < mGlyphCount; i++) {
        if (mGlyphs[i].value == codepoint) {
            index = i;
            break;
        }
        else if (mGlyphs[i].value == FALLBACK) {
            fallbackIndex = i;
        }
    }

    if (!index && mGlyphs[0].value != codepoint) {
        index = fallbackIndex;
    }

    return index;
}

bool HP_Font::generateFontAtlas(HP_Image* atlas, const uint8_t* fileData, int dataSize,
                                HP_FontType fontType, int baseSize,
                                const int* codepoints, int codepointCount,
                                int padding, Glyph** outGlyphs)
{
    assert(atlas != NULL && outGlyphs != NULL);

    /* --- Some basic initialization --- */

    constexpr FT_Int32 ftGlyphFlags =
        FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT | FT_LOAD_TARGET_NORMAL;

    FT_Render_Mode ftRenderMode = (fontType == HP_FONT_SDF)
        ? FT_RENDER_MODE_SDF : FT_RENDER_MODE_NORMAL;

    *outGlyphs = NULL;

    /* --- Font validation and init --- */

    if (!fileData) {
        return false;
    }

    FT_Library ftLibrary;
    if (FT_Init_FreeType(&ftLibrary) != 0) {
        return false;
    }

    FT_Face ftFace;
    if (FT_New_Memory_Face(ftLibrary, fileData, dataSize, 0, &ftFace) != 0) {
        FT_Done_FreeType(ftLibrary);
        return false;
    }

    if (FT_Set_Pixel_Sizes(ftFace, 0, baseSize) != 0) {
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        return false;
    }

    /* --- Get font metrics --- */

    int ascent  = ftFace->size->metrics.ascender  >> 6;
  //int descent = ftFace->size->metrics.descender >> 6;
  //int lineGap = (ftFace->size->metrics.height >> 6) - (ascent - descent);

    /* --- Generate default codepoints if needed --- */

    int* defaultCodepoints = NULL;
    if (!codepoints) {
        codepointCount = (codepointCount > 0) ? codepointCount : 95;
        defaultCodepoints = static_cast<int*>(SDL_malloc(codepointCount * sizeof(int)));
        if (!defaultCodepoints) {
            FT_Done_Face(ftFace);
            FT_Done_FreeType(ftLibrary);
            return false;
        }
        // Generate ASCII printable characters (32-126)
        for (int i = 0; i < codepointCount; i++) {
            defaultCodepoints[i] = i + 32;
        }
        codepoints = defaultCodepoints;
    }

    /* --- Allocate working buffers --- */

    stbrp_rect* packRects = static_cast<stbrp_rect*>(SDL_calloc(codepointCount, sizeof(stbrp_rect)));
    if (!packRects) {
        SDL_free(defaultCodepoints);
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        return false;
    }

    Glyph* glyphs = reinterpret_cast<Glyph*>(SDL_calloc(codepointCount, sizeof(Glyph)));
    if (!glyphs) {
        SDL_free(packRects);
        SDL_free(defaultCodepoints);
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        return false;
    }

    /* --- Generate glyphs and calculate dimensions --- */

    int totalArea = 0;

    for (int i = 0; i < codepointCount; i++)
    {
        Glyph& glyph = glyphs[i];
        int ch = codepoints[i];
        glyph.value = ch;

        // Get the glyph index and load it
        FT_UInt glyphIndex = FT_Get_Char_Index(ftFace, ch);
        if (glyphIndex == 0) {
            continue;
        }
        if (FT_Load_Glyph(ftFace, glyphIndex, ftGlyphFlags) != 0) {
            continue;
        }

        // Space character
        if (ch == 32) {
            glyph.xAdvance = (int)(ftFace->glyph->advance.x >> 6);
            glyph.xOffset = glyph.yOffset = 0;
            glyph.wGlyph = glyph.xAdvance;
            glyph.hGlyph = baseSize;
        }

        // Regular character
        else {
            // Rendering the glyph
            if (FT_Render_Glyph(ftFace->glyph, ftRenderMode) != 0) {
                continue;
            }

            // Get glyph and bitmap references
            const FT_GlyphSlot ftGlyph = ftFace->glyph;
            const FT_Bitmap& ftBitmap = ftGlyph->bitmap;

            // Calculating the number of pixels in the bitmap
            int pixelCount = ftBitmap.width * ftBitmap.rows;
            if (pixelCount == 0) continue;

            // Allocation to keep the bitmap in our glyph cache
            glyph.pixels = SDL_malloc(pixelCount);
            if (!glyph.pixels) continue;

            // Copying the rasterized bitmap to our glyph cache
            SDL_memcpy(glyph.pixels, ftBitmap.buffer, pixelCount);

            // Get horizontal advance
            glyph.xAdvance = (int)(ftGlyph->advance.x >> 6);

            // Calculate the offset needed to draw the glyph
            glyph.xOffset = ftGlyph->bitmap_left;
            glyph.yOffset = ascent - ftGlyph->bitmap_top;

            // Keeps the pixel dimensions of the glyph
            glyph.wGlyph = ftBitmap.width;
            glyph.hGlyph = ftBitmap.rows;
        }

        packRects[i].id = i;
        packRects[i].w = glyph.wGlyph + 2 * padding;
        packRects[i].h = glyph.hGlyph + 2 * padding;
        totalArea += packRects[i].w * packRects[i].h;
    }

    /* --- Calculate Atlas Dimensions --- */

    // NOTE: This naive method is currently the most stable and provides
    //       the best size efficiency across various configurations,
    //       though it can be significantly improved...

    int estimatedArea = (int)(totalArea * 1.3f); // 30% safety margin
    int atlasSize = (int)roundf(sqrtf((float)estimatedArea));

    // Get next po2 if necessary
    if ((atlasSize & (atlasSize - 1)) != 0) {
        --atlasSize;
        atlasSize |= atlasSize >> 1;
        atlasSize |= atlasSize >> 2;
        atlasSize |= atlasSize >> 4;
        atlasSize |= atlasSize >> 8;
        atlasSize |= atlasSize >> 16;
        ++atlasSize;
    }

    // Try rectangle first (wider than tall)
    atlas->w = atlasSize;
    atlas->h = atlasSize / 2;

    // Use square if rectangle is too small
    if (totalArea > atlas->w * atlas->h) {
        atlas->h = atlasSize;
    }

    /* --- Create Atlas Image --- */

    atlas->pixels = SDL_calloc(atlas->w * atlas->h, 1);
    if (!atlas->pixels) {
        for (int i = 0; i < codepointCount; i++) {
            SDL_free(glyphs[i].pixels);
        }
        SDL_free(glyphs);
        SDL_free(packRects);
        SDL_free(defaultCodepoints);
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        return false;
    }
    atlas->format = HP_PIXEL_FORMAT_R8;

    /* --- Rectangle Packing Setup --- */

    stbrp_context* packContext = static_cast<stbrp_context*>(SDL_malloc(sizeof(*packContext)));
    stbrp_node* packNodes = static_cast<stbrp_node*>(SDL_malloc(atlas->w * sizeof(*packNodes)));

    if (!packContext || !packNodes) {
        SDL_free(atlas->pixels);
        for (int i = 0; i < codepointCount; i++) {
            SDL_free(glyphs[i].pixels);
        }
        SDL_free(glyphs);
        SDL_free(packRects);
        SDL_free(packNodes);
        SDL_free(packContext);
        SDL_free(defaultCodepoints);
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        SDL_memset(atlas, 0, sizeof(*atlas));
        return false;
    }

    stbrp_init_target(packContext, atlas->w, atlas->h, packNodes, atlas->w);
    stbrp_pack_rects(packContext, packRects, codepointCount);

    /* --- Copy Stored Glyph Pixels to Atlas --- */

    for (int i = 0; i < codepointCount; i++)
    {
        if (!packRects[i].was_packed) {
            continue;
        }

        Glyph* glyph = &glyphs[i];
        glyph->xAtlas = packRects[i].x + padding;
        glyph->yAtlas = packRects[i].y + padding;

        // Skip spaces and null pixels
        if (!glyph->pixels || glyph->value == 32) {
            continue;
        }

        // Copy glyph to atlas (line by line)
        uint8_t* atlasData = static_cast<uint8_t*>(atlas->pixels);
        uint8_t* glyphData = static_cast<uint8_t*>(glyph->pixels);
        uint8_t* atlasLine = atlasData + glyph->yAtlas * atlas->w + glyph->xAtlas;
        for (int y = 0; y < glyph->hGlyph; y++) {
            SDL_memcpy(atlasLine, glyphData, glyph->wGlyph);
            atlasLine += atlas->w, glyphData += glyph->wGlyph;
        }
    }

    /* --- Keeps the generated glyph array --- */

    *outGlyphs = glyphs;

    /* --- Cleanup --- */

    SDL_free(packRects);
    SDL_free(packNodes);
    SDL_free(packContext);
    SDL_free(defaultCodepoints);
    FT_Done_Face(ftFace);
    FT_Done_FreeType(ftLibrary);

    return true;
}
