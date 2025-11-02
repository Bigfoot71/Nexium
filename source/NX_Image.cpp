/* NX_Image.cpp -- API definition for Nexium's image module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Filesystem.h>
#include <NX/NX_Image.h>
#include <NX/NX_Math.h>
#include <NX/NX_Log.h>

#include <SDL3/SDL_stdinc.h>
#include <fp16.h>

#include <initializer_list>
#include <algorithm>
#include <cstdint>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO

#define STBI_MALLOC(sz)         SDL_malloc(sz)
#define STBI_REALLOC(p,newsz)   SDL_realloc(p,newsz)
#define STBI_FREE(p)            SDL_free(p)

#include <stb_image.h>

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

static NX_PixelFormat INX_GetPixelFormat(int channels, bool isHDR)
{
    if (isHDR) {
        switch (channels) {
        case 1: return NX_PIXEL_FORMAT_R32F;
        case 2: return NX_PIXEL_FORMAT_RG32F;
        case 3: return NX_PIXEL_FORMAT_RGB32F;
        case 4: return NX_PIXEL_FORMAT_RGBA32F;
        }
    }
    else {
        switch (channels) {
        case 1: return NX_PIXEL_FORMAT_R8;
        case 2: return NX_PIXEL_FORMAT_RG8;
        case 3: return NX_PIXEL_FORMAT_RGB8;
        case 4: return NX_PIXEL_FORMAT_RGBA8;
        }
    }
    return NX_PIXEL_FORMAT_INVALID;
}

static NX_Image INX_DecodeImage(const void* data, size_t size, int reqChannels)
{
    NX_Image image{};

    int width, height, channels;
    void* pixels = NULL;
    bool isHDR = false;

    if (stbi_is_hdr_from_memory(static_cast<const stbi_uc*>(data), static_cast<int>(size))) {
        pixels = stbi_loadf_from_memory(
            static_cast<const stbi_uc*>(data), int(size),
            &width, &height, &channels, reqChannels
        );
        isHDR = true;
    }

    if (!pixels) {
        pixels = stbi_load_from_memory(
            static_cast<const stbi_uc*>(data), int(size),
            &width, &height, &channels, reqChannels
        );
    }

    if (!pixels) {
        NX_LOG(E, "IMAGE: Failed to decode image");
        return image;
    }

    int finalChannels = (reqChannels > 0) ? reqChannels : channels;
    NX_PixelFormat format = INX_GetPixelFormat(finalChannels, isHDR);

    if (format == 0) {
        NX_LOG(E, "IMAGE: Unsupported channel count (%d)", finalChannels);
        stbi_image_free(pixels);
        return image;
    }

    image.pixels = pixels;
    image.w = width;
    image.h = height;
    image.format = format;
    
    return image;
}

static NX_PixelFormat INX_GetBestFormat(std::initializer_list<NX_Color> colors, int count)
{
    auto fitsInHalf = [](float f) -> bool {
        return std::abs(f) <= 65504.0f;
    };

    bool hasAlpha = false;
    bool outOfRange = false;
    bool extremeHDR = false;

    for (const NX_Color& c : colors) {
        hasAlpha |= (c.a < 1.0f);
        outOfRange |= (c.r < 0.0f || c.r > 1.0f || c.g < 0.0f || c.g > 1.0f || c.b < 0.0f || c.b > 1.0f);
        extremeHDR |= (outOfRange && !(fitsInHalf(c.r) && fitsInHalf(c.g) && fitsInHalf(c.b) && fitsInHalf(c.a)));
    }

    if (extremeHDR) {
        return hasAlpha ? NX_PIXEL_FORMAT_RGBA32F : NX_PIXEL_FORMAT_RGB32F;
    }

    if (outOfRange) {
        return hasAlpha ? NX_PIXEL_FORMAT_RGBA16F : NX_PIXEL_FORMAT_RGB16F;
    }

    return hasAlpha ? NX_PIXEL_FORMAT_RGBA8 : NX_PIXEL_FORMAT_RGB8;
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Image NX_CreateImage(int w, int h, NX_PixelFormat format)
{
    NX_Image image{};

    if (w <= 0 || h <= 0) {
        return image;
    }

    int bytesPerPixel = NX_GetPixelBytes(format);
    if (bytesPerPixel == 0) {
        return image;
    }

    void* pixels = SDL_calloc(w * h, bytesPerPixel);
    if (!pixels) {
        return image;
    }

    image.pixels = pixels;
    image.w = w;
    image.h = h;
    image.format = format;

    return image;
}

NX_Image NX_CreateImageFromData(const void* pixels, int w, int h, NX_PixelFormat srcFormat, NX_PixelFormat dstFormat)
{
    NX_Image image{};

    if (pixels == NULL || w <= 0 || h <= 0) {
        return image;
    }

    size_t size = w * h;
    size_t dstBpp = NX_GetPixelBytes(dstFormat);

    void* dstPixels = SDL_malloc(size * dstBpp);
    if (dstPixels == NULL) {
        NX_LOG(E, "IMAGE: failed to allocate %zu bytes for image creation from memory", size * dstBpp);
        return image;
    }

    if (srcFormat == dstFormat) {
        SDL_memcpy(dstPixels, pixels, size * dstBpp);
    }
    else {
        for (int i = 0; i < size; i++) {
            NX_Color color = NX_ReadPixel(pixels, i, srcFormat);
            NX_WritePixel(dstPixels, i, dstFormat, color);
        }
    }

    image.pixels = dstPixels;
    image.format = dstFormat;
    image.w = w;
    image.h = h;

    return image;
}

NX_Image NX_LoadImageFromData(const void* data, size_t size)
{
    int channels;
    stbi_info_from_memory((const unsigned char*)data, size, NULL, NULL, &channels);

    return INX_DecodeImage(data, size, (channels == 1) ? 3 : (channels == 2) ? 4 : channels);
}

NX_Image NX_LoadImageRawFromData(const void* data, size_t size)
{
    return INX_DecodeImage(data, size, 0);
}

NX_Image NX_LoadImage(const char* filePath)
{
    NX_Image image{};
    if (!filePath) {
        NX_LOG(E, "IMAGE: File path is null");
        return image;
    }
    
    size_t fileSize;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    if (!fileData) {
        NX_LOG(E, "IMAGE: Failed to load file: %s", filePath);
        return image;
    }
    
    image = NX_LoadImageFromData(fileData, fileSize);
    if (image.pixels == NULL) {
        NX_LOG(E, "IMAGE: Failed to load image: %s", filePath);
    }
    
    return image;
}

NX_Image NX_LoadImageRaw(const char* filePath)
{
    NX_Image image{};
    if (!filePath) {
        NX_LOG(E, "IMAGE: File path is null");
        return image;
    }
    
    size_t fileSize;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    if (!fileData) {
        NX_LOG(E, "IMAGE: Failed to load file: %s", filePath);
        return image;
    }
    
    image = NX_LoadImageRawFromData(fileData, fileSize);
    if (image.pixels == NULL) {
        NX_LOG(E, "IMAGE: Failed to load image: %s", filePath);
    }
    
    return image;
}

void NX_DestroyImage(NX_Image* image)
{
    if (!image || !image->pixels) {
        return;
    }

    SDL_free(image->pixels);
}

NX_Image NX_GenImageColor(int w, int h, NX_Color color)
{
    NX_PixelFormat format = INX_GetBestFormat({ color }, 1);
    size_t bpp = NX_GetPixelBytes(format);

    NX_Image image = NX_CreateImage(w, h, format);
    if (!image.pixels) {
        return image;
    }

    NX_WritePixel(image.pixels, 0, image.format, color);
    uint8_t* pixels = static_cast<uint8_t*>(image.pixels);
    for (size_t i = 1; i < static_cast<size_t>(w) * h; i++) {
        SDL_memcpy(pixels + i * bpp, pixels, bpp);
    }

    return image;
}

NX_Image NX_GenImageGradientLinear(int w, int h, int direction, NX_Color start, NX_Color end)
{
    NX_PixelFormat format = INX_GetBestFormat({ start, end }, 2);
    NX_Image image = NX_CreateImage(w, h, format);
    if (!image.pixels) {
        return image;
    }

    float invW = 1.0f / (w - 1);
    float invH = 1.0f / (h - 1);
    float invD = 1.0f / (w + h - 2);

    int dr = end.r - start.r;
    int dg = end.g - start.g;
    int db = end.b - start.b;
    int da = end.a - start.a;

    switch (direction) {
    case 0:
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float factor = x * invW;
                NX_Color color = {
                    start.r + static_cast<int>(dr * factor),
                    start.g + static_cast<int>(dg * factor),
                    start.b + static_cast<int>(db * factor),
                    start.a + static_cast<int>(da * factor)
                };
                NX_WritePixel(image.pixels, y * w + x, image.format, color);
            }
        }
        break;
    case 1:
        for (int y = 0; y < h; y++) {
            float factor = y * invH;
            NX_Color line_color = {
                start.r + static_cast<int>(dr * factor),
                start.g + static_cast<int>(dg * factor),
                start.b + static_cast<int>(db * factor),
                start.a + static_cast<int>(da * factor)
            };

            for (int x = 0; x < w; x++) {
                NX_WritePixel(image.pixels, y * w + x, image.format, line_color);
            }
        }
        break;
    case 2:
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float factor = (x + y) * invD;

                NX_Color color = {
                    start.r + static_cast<int>(dr * factor),
                    start.g + static_cast<int>(dg * factor),
                    start.b + static_cast<int>(db * factor),
                    start.a + static_cast<int>(da * factor)
                };

                NX_WritePixel(image.pixels, y * w + x, image.format, color);
            }
        }
        break;
    default:
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float factor = x * invW;

                NX_Color color = {
                    start.r + static_cast<int>(dr * factor),
                    start.g + static_cast<int>(dg * factor),
                    start.b + static_cast<int>(db * factor),
                    start.a + static_cast<int>(da * factor)
                };

                NX_WritePixel(image.pixels, y * w + x, image.format, color);
            }
        }
        break;
    }

    return image;
}

NX_Image NX_GenImageGradientRadial(int w, int h, float density, NX_Color inner, NX_Color outer)
{
    NX_PixelFormat format = INX_GetBestFormat({ inner, outer }, 2);
    NX_Image image = NX_CreateImage(w, h, format);
    if (!image.pixels) {
        return image;
    }

    float centerX = w * 0.5f;
    float centerY = h * 0.5f;
    float maxRadius = std::sqrt(centerX * centerX + centerY * centerY) * density;
    float maxRadiusInv = 1.0f / maxRadius;

    float dr = outer.r - inner.r;
    float dg = outer.g - inner.g;
    float db = outer.b - inner.b;
    float da = outer.a - inner.a;

    for (int y = 0; y < h; y++) {
        float dy = y - centerY;
        float dy_squared = dy * dy;
        for (int x = 0; x < w; x++) {
            float dx = x - centerX;
            float distanceSquared = dx * dx + dy_squared;
            float distance = std::sqrt(distanceSquared);
            float factor = distance * maxRadiusInv;
            if (factor > 1.0f) factor = 1.0f;
            NX_Color color = {
                inner.r + static_cast<int>(dr * factor),
                inner.g + static_cast<int>(dg * factor),
                inner.b + static_cast<int>(db * factor),
                inner.a + static_cast<int>(da * factor)
            };
            NX_WritePixel(image.pixels, y * w + x, image.format, color);
        }
    }

    return image;
}

NX_Image NX_GenImageGradientSquare(int w, int h, float density, NX_Color inner, NX_Color outer)
{
    NX_PixelFormat format = INX_GetBestFormat({ inner, outer }, 2);
    NX_Image image = NX_CreateImage(w, h, format);
    if (!image.pixels) {
        return image;
    }

    float centerX = w * 0.5f;
    float centerY = h * 0.5f;
    float maxDistance = std::max(centerX, centerY) * density;
    float maxDistanceInv = 1.0f / maxDistance;

    float dr = outer.r - inner.r;
    float dg = outer.g - inner.g;
    float db = outer.b - inner.b;
    float da = outer.a - inner.a;

    for (int y = 0; y < h; y++) {
        float dy = std::abs(y - centerY);
        for (int x = 0; x < w; x++) {
            float dx = std::abs(x - centerX);
            float distance = (dx > dy) ? dx : dy;
            float factor = distance * maxDistanceInv;
            if (factor > 1.0f) factor = 1.0f;
            NX_Color color = {
                inner.r + static_cast<int>(dr * factor),
                inner.g + static_cast<int>(dg * factor),
                inner.b + static_cast<int>(db * factor),
                inner.a + static_cast<int>(da * factor)
            };
            NX_WritePixel(image.pixels, y * w + x, image.format, color);
        }
    }

    return image;
}

NX_Image NX_GenImageChecked(int w, int h, int xChecks, int yChecks, NX_Color c0, NX_Color c1)
{
    NX_PixelFormat format = INX_GetBestFormat({ c0, c1 }, 2);
    NX_Image image = NX_CreateImage(w, h, format);
    if (!image.pixels) {
        return image;
    }

    uint32_t p0, p1;
    NX_WritePixel(&p0, 0, image.format, c0);
    NX_WritePixel(&p1, 0, image.format, c1);

    for (int y = 0; y < h; y++) {
        int checkY = (y * yChecks) / h;
        for (int x = 0; x < w; x++) {
            int checkX = (x * xChecks) / w;
            static_cast<uint32_t*>(image.pixels)[y * w + x] =
                ((checkX + checkY) % 2 == 0) ? p0 : p1;
        }
    }

    return image;
}

NX_Image NX_CopyImage(const NX_Image* image, NX_PixelFormat format)
{
    NX_Image result{};

    if (image == NULL || image->pixels == NULL || image->format == format) {
        return result;
    }

    size_t size = image->w * image->h;
    size_t bpp = NX_GetPixelBytes(format);

    void* pixels = SDL_malloc(size * bpp);
    if (pixels == NULL) {
        NX_LOG(E, "IMAGE: failed to allocate %zu bytes for image conversion", size * bpp);
        return result;
    }

    if (image->format == format) {
        SDL_memcpy(pixels, image->pixels, size * bpp);
    }
    else {
        for (int i = 0; i < size; i++) {
            NX_Color color = NX_ReadPixel(image->pixels, i, image->format);
            NX_WritePixel(pixels, i, format, color);
        }
    }

    result.pixels = pixels;
    result.format = format;
    result.w = image->w;
    result.h = image->h;

    return result;
}

NX_Image NX_ComposeImagesRGB(const NX_Image* sources[3], NX_Color defaultColor)
{
    NX_Image image{};

    /* --- Determine dimensions --- */

    int w = 0, h = 0;
    for (int i = 0; i < 3; i++) {
        if (sources[i]) {
            w = std::max(w, sources[i]->w);
            h = std::max(h, sources[i]->h);
        }
    }

    if (w == 0 || h == 0) {
        return image;
    }

    /* --- Calculate scales --- */

    int scaleX[3], scaleY[3];
    for (int i = 0; i < 3; i++) {
        if (sources[i] && sources[i]->w > 0 && sources[i]->h > 0) {
            scaleX[i] = (sources[i]->w << 16) / w;
            scaleY[i] = (sources[i]->h << 16) / h;
        }
    }

    /* --- Allocation --- */

    image = NX_CreateImage(w, h, NX_PIXEL_FORMAT_RGB8);
    if (!image.pixels) {
        return image;
    }

    /* --- Generate optimized loop based on source combination --- */

    int mask = (sources[0] ? 1 : 0) | (sources[1] ? 2 : 0) | (sources[2] ? 4 : 0);

    #define SAMPLE_TEMPLATE(R_CODE, G_CODE, B_CODE) \
        for (int y = 0; y < h; y++) { \
            for (int x = 0; x < w; x++) { \
                NX_Color color = defaultColor; \
                R_CODE; G_CODE; B_CODE; \
                NX_WritePixel(image.pixels, y * w + x, image.format, color); \
            } \
        }

    #define SAMPLE_R do { \
        int srcX = (x * scaleX[0]) >> 16; \
        int srcY = (y * scaleY[0]) >> 16; \
        if (srcX >= sources[0]->w) srcX = sources[0]->w - 1; \
        if (srcY >= sources[0]->h) srcY = sources[0]->h - 1; \
        NX_Color src = NX_ReadPixel(sources[0]->pixels, srcY * sources[0]->w + srcX, sources[0]->format); \
        color.r = src.r; \
    } while(0)

    #define SAMPLE_G do { \
        int srcX = (x * scaleX[1]) >> 16; \
        int srcY = (y * scaleY[1]) >> 16; \
        if (srcX >= sources[1]->w) srcX = sources[1]->w - 1; \
        if (srcY >= sources[1]->h) srcY = sources[1]->h - 1; \
        NX_Color src = NX_ReadPixel(sources[1]->pixels, srcY * sources[1]->w + srcX, sources[1]->format); \
        color.g = src.g; \
    } while(0)

    #define SAMPLE_B do { \
        int srcX = (x * scaleX[2]) >> 16; \
        int srcY = (y * scaleY[2]) >> 16; \
        if (srcX >= sources[2]->w) srcX = sources[2]->w - 1; \
        if (srcY >= sources[2]->h) srcY = sources[2]->h - 1; \
        NX_Color src = NX_ReadPixel(sources[2]->pixels, srcY * sources[2]->w + srcX, sources[2]->format); \
        color.b = src.b; \
    } while(0)

    #define NOOP do {} while(0)

    switch (mask) {
        case 0: break; // No sources
        case 1: SAMPLE_TEMPLATE(SAMPLE_R, NOOP, NOOP); break;
        case 2: SAMPLE_TEMPLATE(NOOP, SAMPLE_G, NOOP); break;
        case 3: SAMPLE_TEMPLATE(SAMPLE_R, SAMPLE_G, NOOP); break;
        case 4: SAMPLE_TEMPLATE(NOOP, NOOP, SAMPLE_B); break;
        case 5: SAMPLE_TEMPLATE(SAMPLE_R, NOOP, SAMPLE_B); break;
        case 6: SAMPLE_TEMPLATE(NOOP, SAMPLE_G, SAMPLE_B); break;
        case 7: SAMPLE_TEMPLATE(SAMPLE_R, SAMPLE_G, SAMPLE_B); break;
    }

    #undef SAMPLE_TEMPLATE
    #undef SAMPLE_R
    #undef SAMPLE_G
    #undef SAMPLE_B
    #undef NOOP

    return image;
}

void NX_SetImagePixel(const NX_Image* image, int x, int y, NX_Color color)
{
    if (image && image->pixels && x >= 0 && x < image->w && y >= 0 && y < image->h) {
        NX_WritePixel(image->pixels, y * image->w + x, image->format, color);
    }
}

NX_Color NX_GetImagePixel(const NX_Image* image, int x, int y)
{
    if (image && image->pixels && x >= 0 && x < image->w && y >= 0 && y < image->h) {
        return NX_ReadPixel(image->pixels, y * image->w + x, image->format);
    }
    return NX_BLANK;
}

void NX_ConvertImage(NX_Image* image, NX_PixelFormat format)
{
    if (image == NULL || image->pixels == NULL || image->format == format) {
        return;
    }

    size_t size = image->w * image->h;
    size_t bpp = NX_GetPixelBytes(format);

    void* pixels = SDL_malloc(size * bpp);
    if (pixels == NULL) {
        NX_LOG(E, "IMAGE: failed to allocate %zu bytes for image conversion", size * bpp);
        return;
    }

    for (int i = 0; i < size; i++) {
        NX_Color color = NX_ReadPixel(image->pixels, i, image->format);
        NX_WritePixel(pixels, i, format, color);
    }

    SDL_free(image->pixels);

    image->pixels = pixels;
    image->format = format;
}

void NX_InvertImage(const NX_Image* image)
{
    if (image == NULL || image->pixels == NULL) {
        return;
    }

    NX_PixelFormat format = image->format;
    void* pixels = image->pixels;

    int size = image->w * image->h;

    for (int i = 0; i < size; i++) {
        NX_Color color = NX_ReadPixel(pixels, i, format);
        color.r = 1.0f - color.r;
        color.g = 1.0f - color.g;
        color.b = 1.0f - color.b;
        NX_WritePixel(pixels, i, format, color);
    }
}

void NX_BlitImage(
    const NX_Image* src, int srcX, int srcY, int srcW, int srcH,
    const NX_Image* dst, int dstX, int dstY, int dstW, int dstH)
{
    if (!src || !dst || !src->pixels || !dst->pixels ||
        srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0) {
        return;
    }

    if (srcX < 0) { srcW += srcX; srcX = 0; }
    if (srcY < 0) { srcH += srcY; srcY = 0; }
    if (srcX + srcW > src->w) srcW = src->w - srcX;
    if (srcY + srcH > src->h) srcH = src->h - srcY;

    int clipDstX = std::max(0, dstX);
    int clipDstY = std::max(0, dstY);
    int clipDstW = std::min(dstW, dst->w - clipDstX);
    int clipDstH = std::min(dstH, dst->h - clipDstY);

    if (srcW <= 0 || srcH <= 0 || clipDstW <= 0 || clipDstH <= 0) {
        return;
    }

    int scaleX = (srcW << 16) / dstW;
    int scaleY = (srcH << 16) / dstH;

    int startOffsetX = clipDstX - dstX;
    int startOffsetY = clipDstY - dstY;
    int srcStartX = srcX + ((startOffsetX * scaleX) >> 16);
    int srcStartY = srcY + ((startOffsetY * scaleY) >> 16);

    for (int y = 0; y < clipDstH; y++) {
        for (int x = 0; x < clipDstW; x++)
        {
            int srcPixelX = srcStartX + ((x * scaleX) >> 16);
            int srcPixelY = srcStartY + ((y * scaleY) >> 16);

            if (srcPixelX >= src->w) srcPixelX = src->w - 1;
            if (srcPixelY >= src->h) srcPixelY = src->h - 1;

            int srcIndex = srcPixelY * src->w + srcPixelX;
            int dstIndex = (clipDstY + y) * dst->w + (clipDstX + x);

            NX_Color color = NX_ReadPixel(src->pixels, srcIndex, src->format);
            NX_WritePixel(dst->pixels, dstIndex, dst->format, color);
        }
    }
}

int NX_GetPixelBytes(NX_PixelFormat format)
{
    switch (format) {
    case NX_PIXEL_FORMAT_R8:        return 1;
    case NX_PIXEL_FORMAT_RG8:       return 2;
    case NX_PIXEL_FORMAT_RGB8:      return 3;
    case NX_PIXEL_FORMAT_RGBA8:     return 4;
    case NX_PIXEL_FORMAT_R16F:      return 2;
    case NX_PIXEL_FORMAT_RG16F:     return 4;
    case NX_PIXEL_FORMAT_RGB16F:    return 6;
    case NX_PIXEL_FORMAT_RGBA16F:   return 8;
    case NX_PIXEL_FORMAT_R32F:      return 4;
    case NX_PIXEL_FORMAT_RG32F:     return 8;
    case NX_PIXEL_FORMAT_RGB32F:    return 12;
    case NX_PIXEL_FORMAT_RGBA32F:   return 16;
    default: break;
    }
    return 0;
}

int NX_GetPixelChannels(NX_PixelFormat format)
{
    switch (format) {
    case NX_PIXEL_FORMAT_R8:
    case NX_PIXEL_FORMAT_R16F:
    case NX_PIXEL_FORMAT_R32F:
        return 1;
    case NX_PIXEL_FORMAT_RG8:
    case NX_PIXEL_FORMAT_RG16F:
    case NX_PIXEL_FORMAT_RG32F:
        return 2;
    case NX_PIXEL_FORMAT_RGB8:
    case NX_PIXEL_FORMAT_RGB16F:
    case NX_PIXEL_FORMAT_RGB32F:
        return 3;
    case NX_PIXEL_FORMAT_RGBA8:
    case NX_PIXEL_FORMAT_RGBA16F:
    case NX_PIXEL_FORMAT_RGBA32F:
        return 4;
    default:
        break;
    }
    return 0;
}

int NX_GetPixelChannelBytes(NX_PixelFormat format)
{
    switch (format) {
    case NX_PIXEL_FORMAT_R8:
    case NX_PIXEL_FORMAT_RG8:
    case NX_PIXEL_FORMAT_RGB8:
    case NX_PIXEL_FORMAT_RGBA8:
        return 1;
    case NX_PIXEL_FORMAT_R16F:
    case NX_PIXEL_FORMAT_RG16F:
    case NX_PIXEL_FORMAT_RGB16F:
    case NX_PIXEL_FORMAT_RGBA16F:
        return 2;
    case NX_PIXEL_FORMAT_R32F:
    case NX_PIXEL_FORMAT_RG32F:
    case NX_PIXEL_FORMAT_RGB32F:
    case NX_PIXEL_FORMAT_RGBA32F:
        return 4;
    default:
        break;
    }
    return 0;
}

void NX_WritePixel(void* pixels, int index, NX_PixelFormat format, NX_Color color)
{
    switch (format) {
    case NX_PIXEL_FORMAT_R8:
        {
            uint8_t* p = static_cast<uint8_t*>(pixels);
            p[index] = static_cast<uint8_t>(NX_Saturate(color.r) * 255.0f);
        }
        break;
    case NX_PIXEL_FORMAT_RG8:
        {
            uint8_t* p = static_cast<uint8_t*>(pixels);
            p[index * 2 + 0] = static_cast<uint8_t>(NX_Saturate(color.r) * 255.0f);
            p[index * 2 + 1] = static_cast<uint8_t>(NX_Saturate(color.g) * 255.0f);
        }
        break;
    case NX_PIXEL_FORMAT_RGB8:
        {
            uint8_t* p = static_cast<uint8_t*>(pixels);
            p[index * 3 + 0] = static_cast<uint8_t>(NX_Saturate(color.r) * 255.0f);
            p[index * 3 + 1] = static_cast<uint8_t>(NX_Saturate(color.g) * 255.0f);
            p[index * 3 + 2] = static_cast<uint8_t>(NX_Saturate(color.b) * 255.0f);
        }
        break;
    case NX_PIXEL_FORMAT_RGBA8:
        {
            uint8_t* p = static_cast<uint8_t*>(pixels);
            p[index * 4 + 0] = static_cast<uint8_t>(NX_Saturate(color.r) * 255.0f);
            p[index * 4 + 1] = static_cast<uint8_t>(NX_Saturate(color.g) * 255.0f);
            p[index * 4 + 2] = static_cast<uint8_t>(NX_Saturate(color.b) * 255.0f);
            p[index * 4 + 3] = static_cast<uint8_t>(NX_Saturate(color.a) * 255.0f);
        }
        break;
    case NX_PIXEL_FORMAT_R16F:
        {
            uint16_t* p = static_cast<uint16_t*>(pixels);
            p[index] = fp16_ieee_from_fp32_value(NX_CLAMP(color.r, -65504.0f, 65504.0f));
        }
        break;
    case NX_PIXEL_FORMAT_RG16F:
        {
            uint16_t* p = static_cast<uint16_t*>(pixels);
            p[index * 2 + 0] = fp16_ieee_from_fp32_value(NX_CLAMP(color.r, -65504.0f, 65504.0f));
            p[index * 2 + 1] = fp16_ieee_from_fp32_value(NX_CLAMP(color.g, -65504.0f, 65504.0f));
        }
        break;
    case NX_PIXEL_FORMAT_RGB16F:
        {
            uint16_t* p = static_cast<uint16_t*>(pixels);
            p[index * 3 + 0] = fp16_ieee_from_fp32_value(NX_CLAMP(color.r, -65504.0f, 65504.0f));
            p[index * 3 + 1] = fp16_ieee_from_fp32_value(NX_CLAMP(color.g, -65504.0f, 65504.0f));
            p[index * 3 + 2] = fp16_ieee_from_fp32_value(NX_CLAMP(color.b, -65504.0f, 65504.0f));
        }
        break;
    case NX_PIXEL_FORMAT_RGBA16F:
        {
            uint16_t* p = static_cast<uint16_t*>(pixels);
            p[index * 4 + 0] = fp16_ieee_from_fp32_value(NX_CLAMP(color.r, -65504.0f, 65504.0f));
            p[index * 4 + 1] = fp16_ieee_from_fp32_value(NX_CLAMP(color.g, -65504.0f, 65504.0f));
            p[index * 4 + 2] = fp16_ieee_from_fp32_value(NX_CLAMP(color.b, -65504.0f, 65504.0f));
            p[index * 4 + 3] = fp16_ieee_from_fp32_value(NX_CLAMP(color.a, -65504.0f, 65504.0f));
        }
        break;
    case NX_PIXEL_FORMAT_R32F:
        {
            float* p = static_cast<float*>(pixels);
            p[index] = color.r;
        }
        break;
    case NX_PIXEL_FORMAT_RG32F:
        {
            float* p = static_cast<float*>(pixels);
            p[index * 2 + 0] = color.r;
            p[index * 2 + 1] = color.g;
        }
        break;
    case NX_PIXEL_FORMAT_RGB32F:
        {
            float* p = static_cast<float*>(pixels);
            p[index * 3 + 0] = color.r;
            p[index * 3 + 1] = color.g;
            p[index * 3 + 2] = color.b;
        }
        break;
    case NX_PIXEL_FORMAT_RGBA32F:
        {
            float* p = static_cast<float*>(pixels);
            p[index * 4 + 0] = color.r;
            p[index * 4 + 1] = color.g;
            p[index * 4 + 2] = color.b;
            p[index * 4 + 3] = color.a;
        }
        break;
    case NX_PIXEL_FORMAT_INVALID:
        break;
    }
}

NX_Color NX_ReadPixel(const void* pixels, int index, NX_PixelFormat format)
{
    NX_Color color = NX_BLACK;

    switch (format) {
    case NX_PIXEL_FORMAT_R8:
        {
            const uint8_t* p = static_cast<const uint8_t*>(pixels);
            color.r = p[index] / 255.0f;
        }
        break;
    case NX_PIXEL_FORMAT_RG8:
        {
            const uint8_t* p = static_cast<const uint8_t*>(pixels);
            color.r = p[index * 2 + 0] / 255.0f;
            color.g = p[index * 2 + 1] / 255.0f;
        }
        break;
    case NX_PIXEL_FORMAT_RGB8:
        {
            const uint8_t* p = static_cast<const uint8_t*>(pixels);
            color.r = p[index * 3 + 0] / 255.0f;
            color.g = p[index * 3 + 1] / 255.0f;
            color.b = p[index * 3 + 2] / 255.0f;
        }
        break;
    case NX_PIXEL_FORMAT_RGBA8:
        {
            const uint8_t* p = static_cast<const uint8_t*>(pixels);
            color.r = p[index * 4 + 0] / 255.0f;
            color.g = p[index * 4 + 1] / 255.0f;
            color.b = p[index * 4 + 2] / 255.0f;
            color.a = p[index * 4 + 3] / 255.0f;
        }
        break;
    case NX_PIXEL_FORMAT_R16F:
        {
            const uint16_t* p = static_cast<const uint16_t*>(pixels);
            color.r = fp16_ieee_to_fp32_value(p[index]);
        }
        break;
    case NX_PIXEL_FORMAT_RG16F:
        {
            const uint16_t* p = static_cast<const uint16_t*>(pixels);
            color.r = fp16_ieee_to_fp32_value(p[index * 2 + 0]);
            color.g = fp16_ieee_to_fp32_value(p[index * 2 + 1]);
        }
        break;
    case NX_PIXEL_FORMAT_RGB16F:
        {
            const uint16_t* p = static_cast<const uint16_t*>(pixels);
            color.r = fp16_ieee_to_fp32_value(p[index * 3 + 0]);
            color.g = fp16_ieee_to_fp32_value(p[index * 3 + 1]);
            color.b = fp16_ieee_to_fp32_value(p[index * 3 + 2]);
        }
        break;
    case NX_PIXEL_FORMAT_RGBA16F:
        {
            const uint16_t* p = static_cast<const uint16_t*>(pixels);
            color.r = fp16_ieee_to_fp32_value(p[index * 4 + 0]);
            color.g = fp16_ieee_to_fp32_value(p[index * 4 + 1]);
            color.b = fp16_ieee_to_fp32_value(p[index * 4 + 2]);
            color.a = fp16_ieee_to_fp32_value(p[index * 4 + 3]);
        }
        break;
    case NX_PIXEL_FORMAT_R32F:
        {
            const float* p = static_cast<const float*>(pixels);
            color.r = p[index];
        }
        break;
    case NX_PIXEL_FORMAT_RG32F:
        {
            const float* p = static_cast<const float*>(pixels);
            color.r = p[index * 2 + 0];
            color.g = p[index * 2 + 1];
        }
        break;
    case NX_PIXEL_FORMAT_RGB32F:
        {
            const float* p = static_cast<const float*>(pixels);
            color.r = p[index * 3 + 0];
            color.g = p[index * 3 + 1];
            color.b = p[index * 3 + 2];
        }
        break;
    case NX_PIXEL_FORMAT_RGBA32F:
        {
            const float* p = static_cast<const float*>(pixels);
            color.r = p[index * 4 + 0];
            color.g = p[index * 4 + 1];
            color.b = p[index * 4 + 2];
            color.a = p[index * 4 + 3];
        }
        break;
    case NX_PIXEL_FORMAT_INVALID:
        break;
    }

    return color;
}
