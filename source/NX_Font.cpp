/* NX_Font.cpp -- Implementation of the API for fonts
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Font.hpp"

#include "./Detail/Util/FixedArray.hpp"
#include "./Detail/Util/Memory.hpp"

#include "./INX_GlobalAssets.hpp"
#include "./INX_GlobalPool.hpp"

#include <NX/NX_Filesystem.h>
#include <NX/NX_Codepoint.h>
#include <NX/NX_Texture.h>
#include <NX/NX_Memory.h>
#include <NX/NX_Font.h>
#include <NX/NX_Log.h>

// ============================================================================
// FREETYPE INCLUDES
// ============================================================================

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// ============================================================================
// STB RECT PACK IMPLEMENTATION
// ============================================================================

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

NX_Font::~NX_Font()
{
    NX_DestroyTexture(texture);
}

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

static int INX_GetGlyphIndex(const NX_Font* font, int codepoint);

static bool INX_GenerateAtlas(NX_Image* atlas, const uint8_t* fileData, int dataSize,
                              NX_FontType fontType, int baseSize, const int* codepoints,
                              int codepointCount, int padding, util::FixedArray<INX_Glyph>* outGlyphs);

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Font* NX_LoadFont(const char* filePath, NX_FontType type, int baseSize, const int* codepoints, int codepointCount)
{
    size_t dataSize = 0;
    void* fileData = NX_LoadFile(filePath, &dataSize);

    NX_Font* font = NX_LoadFontFromData(fileData, dataSize, type, baseSize, codepoints, codepointCount);
    NX_Free(fileData);

    return font;
}

NX_Font* NX_LoadFontFromData(const void* fileData, size_t dataSize, NX_FontType type, int baseSize, const int* codepoints, int codepointCount)
{
#   define FONT_TTF_DEFAULT_SIZE           32
#   define FONT_TTF_DEFAULT_NUMCHARS       95
#   define FONT_TTF_DEFAULT_FIRST_CHAR     32
#   define FONT_TTF_DEFAULT_CHARS_PADDING   4

    /* --- Base configuration --- */

    codepointCount = (codepointCount > 0) ? codepointCount : FONT_TTF_DEFAULT_NUMCHARS;

    /* --- Generation of the atlas image --- */

    util::FixedArray<INX_Glyph> glyphs{};
    NX_Image atlas{};

    bool atlasGenerated = INX_GenerateAtlas(
        &atlas,
        static_cast<const uint8_t*>(fileData),
        dataSize,
        type,
        baseSize,
        codepoints,
        codepointCount,
        FONT_TTF_DEFAULT_CHARS_PADDING,
        &glyphs
    );

    if (!atlasGenerated) {
        NX_LOG(E, "RENDER: Failed to generate font atlas");
        return nullptr;
    }

    /* --- Creating the atlas texture --- */

    NX_TextureFilter filter = (type == NX_FONT_MONO) ? NX_TEXTURE_FILTER_POINT : NX_TEXTURE_FILTER_BILINEAR;
    NX_Texture* texture = NX_CreateTextureFromImageEx(&atlas, NX_TEXTURE_WRAP_CLAMP, filter);
    NX_DestroyImage(&atlas);

    if (texture == nullptr) {
        NX_LOG(E, "RENDER: Failed to create font atlas texture");
        return nullptr;
    }

    /* --- Returns object pushed into the pool --- */

    NX_Font* font = INX_Pool.Create<NX_Font>();

    font->baseSize = baseSize;
    font->glyphPadding = FONT_TTF_DEFAULT_CHARS_PADDING;
    font->texture = texture;
    font->glyphs = std::move(glyphs);
    font->type = type;

    return font;
}

void NX_DestroyFont(NX_Font* font)
{
    INX_Pool.Destroy(font);
}

NX_FontType NX_GetFontType(const NX_Font* font)
{
    return font->type;
}

NX_Vec2 NX_MeasureCodepoints(const NX_Font* font, const int* codepoints, int length, float fontSize, NX_Vec2 spacing)
{
    font = INX_Assets.Select(font, INX_FontAsset::DEFAULT);

    float scale = fontSize / font->baseSize;

    float maxWidth = 0.0f;
    float currentWidth = 0.0f;
    float textHeight = fontSize;

    int maxCharsInLine = 0;
    int currentCharsInLine = 0;

    for (int i = 0; i < length; i++)
    {
        int letter = codepoints[i];

        if (letter == '\n') {
            maxWidth = std::max(maxWidth, currentWidth);
            maxCharsInLine = NX_MAX(maxCharsInLine, currentCharsInLine);
            currentWidth = 0.0f;
            currentCharsInLine = 0;
            textHeight += fontSize + spacing.y;
        }
        else {
            int index = INX_GetGlyphIndex(font, letter);
            float charWith = (font->glyphs[index].xAdvance > 0)
                ? font->glyphs[index].xAdvance
                : (font->glyphs[index].wGlyph + font->glyphs[index].xOffset);

            currentWidth += charWith;
            currentCharsInLine++;
        }
    }

    // Treat the last line
    maxWidth = std::max(maxWidth, currentWidth);
    maxCharsInLine = NX_MAX(maxCharsInLine, currentCharsInLine);

    return NX_Vec2 {
        .x = maxWidth * scale + (maxCharsInLine > 0 ? (maxCharsInLine - 1) * spacing.x : 0),
        .y = textHeight
    };
}

NX_Vec2 NX_MeasureText(const NX_Font* font, const char* text, float fontSize, NX_Vec2 spacing)
{
    font = INX_Assets.Select(font, INX_FontAsset::DEFAULT);

    float scale = fontSize / font->baseSize;

    float maxWidth = 0.0f;
    float currentWidth = 0.0f;
    float textHeight = fontSize;

    int maxCharsInLine = 0;
    int currentCharsInLine = 0;
    int textLength = (int)strlen(text);

    for (int i = 0; i < textLength;)
    {
        int codepoint_byte_count = 0;
        int letter = NX_GetCodepointNext(&text[i], &codepoint_byte_count);
        i += codepoint_byte_count;

        if (letter == '\n') {
            maxWidth = std::max(maxWidth, currentWidth);
            maxCharsInLine = NX_MAX(maxCharsInLine, currentCharsInLine);
            currentWidth = 0.0f;
            currentCharsInLine = 0;
            textHeight += fontSize + spacing.y;
        }
        else {
            int index = INX_GetGlyphIndex(font, letter);
            float charWith = (font->glyphs[index].xAdvance > 0)
                ? font->glyphs[index].xAdvance
                : (font->glyphs[index].wGlyph + font->glyphs[index].xOffset);

            currentWidth += charWith;
            currentCharsInLine++;
        }
    }

    // Treat the last line
    maxWidth = std::max(maxWidth, currentWidth);
    maxCharsInLine = NX_MAX(maxCharsInLine, currentCharsInLine);

    return NX_Vec2 {
        .x = maxWidth * scale + (maxCharsInLine > 0 ? (maxCharsInLine - 1) * spacing.x : 0),
        .y = textHeight
    };
}

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

int INX_GetGlyphIndex(const NX_Font* font, int codepoint)
{
#   define FALLBACK 63 //< Fallback is '?'

    int index = 0, fallbackIndex = 0;

    for (int i = 0; i < font->glyphs.size(); i++) {
        if (font->glyphs[i].value == codepoint) {
            index = i;
            break;
        }
        else if (font->glyphs[i].value == FALLBACK) {
            fallbackIndex = i;
        }
    }

    if (!index && font->glyphs[0].value != codepoint) {
        index = fallbackIndex;
    }

    return index;
}

const INX_Glyph& INX_GetFontGlyph(const NX_Font* font, int codepoint)
{
    return font->glyphs[INX_GetGlyphIndex(font, codepoint)];
}

bool INX_GenerateAtlas(NX_Image* atlas, const uint8_t* fileData, int dataSize,
                       NX_FontType fontType, int baseSize, const int* codepoints,
                       int codepointCount, int padding, util::FixedArray<INX_Glyph>* outGlyphs)
{
    assert(atlas != NULL && outGlyphs != NULL);

    /* --- Some basic initialization --- */

    FT_Render_Mode ftRenderMode{};
    FT_Int32 ftGlyphFlags = FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT;

    switch (fontType) {
    case NX_FONT_NORMAL:
        ftRenderMode = FT_RENDER_MODE_NORMAL;
        ftGlyphFlags |= FT_LOAD_TARGET_NORMAL;
        break;
    case NX_FONT_LIGHT:
        ftRenderMode = FT_RENDER_MODE_LIGHT;
        ftGlyphFlags |= FT_LOAD_TARGET_LIGHT;
        break;
    case NX_FONT_MONO:
        ftRenderMode = FT_RENDER_MODE_MONO;
        ftGlyphFlags |= FT_LOAD_TARGET_MONO;
        break;
    case NX_FONT_SDF:
        ftRenderMode = FT_RENDER_MODE_SDF;
        ftGlyphFlags |= FT_LOAD_TARGET_NORMAL;
        break;
    default:
        NX_LOG(E, "RENDER: Faild to load font; Invalid font type (%i)", fontType);
        return false;
    }

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

    util::UniquePtr<int> defaultCodepoints;

    if (!codepoints) {
        codepointCount = (codepointCount > 0) ? codepointCount : 95;
        defaultCodepoints = util::MakeUniqueArray<int>(codepointCount);
        if (!defaultCodepoints) {
            FT_Done_Face(ftFace);
            FT_Done_FreeType(ftLibrary);
            return false;
        }
        // Generate ASCII printable characters (32-126)
        for (int i = 0; i < codepointCount; i++) {
            defaultCodepoints.get()[i] = i + 32;
        }
        codepoints = defaultCodepoints.get();
    }

    /* --- Allocate working buffers --- */

    util::UniquePtr<stbrp_rect> packRects = util::MakeUniqueArray<stbrp_rect>(codepointCount);
    if (packRects == nullptr) {
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        return false;
    }

    util::FixedArray<INX_Glyph> glyphs(codepointCount, codepointCount);
    if (glyphs.data() == nullptr) {
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        return false;
    }

    /* --- Generate glyphs and calculate dimensions --- */

    int totalArea = 0;

    for (int i = 0; i < codepointCount; i++)
    {
        INX_Glyph& glyph = glyphs[i];
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
            glyph.pixels = util::MakeUniqueArray<uint8_t>(pixelCount);
            if (glyph.pixels == nullptr) continue;

            // Copying the rasterized bitmap to our glyph cache
            if (fontType != NX_FONT_MONO) {
                SDL_memcpy(glyph.pixels.get(), ftBitmap.buffer, pixelCount);
            }
            else {
                const FT_Bitmap &bm = ftBitmap;
                for (unsigned int y = 0; y < bm.rows; ++y) {
                    for (unsigned int x = 0; x < bm.width; ++x) {
                        int byteIndex = y * bm.pitch + (x >> 3);
                        int bitIndex = 7 - (x & 7);
                        bool pixelOn = (bm.buffer[byteIndex] >> bitIndex) & 1;
                        glyph.pixels.get()[y * bm.width + x] = pixelOn ? 255 : 0;
                    }
                }
            }

            // Get horizontal advance
            glyph.xAdvance = (int)(ftGlyph->advance.x >> 6);

            // Calculate the offset needed to draw the glyph
            glyph.xOffset = ftGlyph->bitmap_left;
            glyph.yOffset = ascent - ftGlyph->bitmap_top;

            // Keeps the pixel dimensions of the glyph
            glyph.wGlyph = ftBitmap.width;
            glyph.hGlyph = ftBitmap.rows;
        }

        packRects.get()[i].id = i;
        packRects.get()[i].w = glyph.wGlyph + 2 * padding;
        packRects.get()[i].h = glyph.hGlyph + 2 * padding;
        totalArea += packRects.get()[i].w * packRects.get()[i].h;
    }

    /* --- Calculate Atlas Dimensions --- */

    // NOTE: This naive method is currently the most stable and provides
    //       the best size efficiency across various configurations,
    //       though it can be significantly improved...

    int estimatedArea = (int)(totalArea * 1.3f); // 30% safety margin
    int atlasSize = (int)roundf(sqrtf((float)estimatedArea));

    // Get next po2 if necessary
    if (!NX_IsPowerOfTwo(atlasSize)) {
        atlasSize = NX_NextPowerOfTwo(atlasSize);
    }

    // Try rectangle first (wider than tall)
    atlas->w = atlasSize;
    atlas->h = atlasSize / 2;

    // Use square if rectangle is too small
    if (totalArea > atlas->w * atlas->h) {
        atlas->h = atlasSize;
    }

    /* --- Create Atlas Image --- */

    atlas->pixels = NX_Calloc(atlas->w * atlas->h, 1);
    if (!atlas->pixels) {
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        return false;
    }
    atlas->format = NX_PIXEL_FORMAT_R8;

    /* --- Rectangle Packing Setup --- */

    util::UniquePtr<stbrp_context> packContext = util::MakeUnique<stbrp_context>();
    util::UniquePtr<stbrp_node> packNodes = util::MakeUniqueArray<stbrp_node>(atlas->w);

    if (!packContext || !packNodes) {
        NX_Free(atlas->pixels);
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLibrary);
        SDL_memset(atlas, 0, sizeof(*atlas));
        return false;
    }

    stbrp_init_target(packContext.get(), atlas->w, atlas->h, packNodes.get(), atlas->w);
    stbrp_pack_rects(packContext.get(), packRects.get(), codepointCount);

    /* --- Copy Stored INX_Glyph Pixels to Atlas --- */

    for (int i = 0; i < codepointCount; i++)
    {
        if (!packRects.get()[i].was_packed) {
            continue;
        }

        INX_Glyph* glyph = &glyphs[i];
        glyph->xAtlas = packRects.get()[i].x + padding;
        glyph->yAtlas = packRects.get()[i].y + padding;

        // Skip spaces and null pixels
        if (!glyph->pixels || glyph->value == 32) {
            continue;
        }

        // Copy glyph to atlas (line by line)
        uint8_t* atlasData = static_cast<uint8_t*>(atlas->pixels);
        uint8_t* glyphData = glyph->pixels.get();
        uint8_t* atlasLine = atlasData + glyph->yAtlas * atlas->w + glyph->xAtlas;
        for (int y = 0; y < glyph->hGlyph; y++) {
            SDL_memcpy(atlasLine, glyphData, glyph->wGlyph);
            atlasLine += atlas->w, glyphData += glyph->wGlyph;
        }
    }

    /* --- Keeps the generated glyph array --- */

    *outGlyphs = std::move(glyphs);

    /* --- Cleanup --- */

    FT_Done_Face(ftFace);
    FT_Done_FreeType(ftLibrary);

    return true;
}
