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

#include "./AssetCache.hpp"

#include <Hyperion/HP_Rand.h>
#include <SDL3/SDL_stdinc.h>
#include <fp16.h>

#include <shaders/screen.vert.h>
#include <shaders/cube.vert.h>

#include <assets/brdf_lut_512_rg16_float.raw.h>
#include <assets/font.ttf.h>

namespace {

/* === Codepoints of the default font === */

// NOTE: Codepoints extracted with:
// - otfinfo -u font.ttf | awk '{print substr($1,4)}'
constexpr int CODEPOINTS[] = {
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x00A0,
    0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8,
    0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0,
    0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8,
    0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF, 0x00C0,
    0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8,
    0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF, 0x00D0,
    0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8,
    0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0,
    0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8,
    0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0,
    0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8,
    0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF, 0x0106,
    0x0107, 0x010C, 0x010D, 0x0111, 0x011E, 0x011F, 0x0130, 0x0131,
    0x0141, 0x0142, 0x0152, 0x0153, 0x015E, 0x015F, 0x0160, 0x0161,
    0x0178, 0x017D, 0x017E, 0x0192, 0x02C6, 0x02C7, 0x02D8, 0x02D9,
    0x02DA, 0x02DB, 0x02DC, 0x02DD, 0x03A9, 0x03C0, 0x2013, 0x2014,
    0x2018, 0x2019, 0x201A, 0x201C, 0x201D, 0x201E, 0x2020, 0x2021,
    0x2022, 0x2026, 0x2030, 0x2039, 0x203A, 0x20AC, 0x2122, 0x2202,
    0x2206, 0x220F, 0x2211, 0x2212, 0x2215, 0x2219, 0x221A, 0x221E,
    0x222B, 0x2248, 0x2260, 0x2264, 0x2265, 0x25CA, 0xFB01, 0xFB02,
    0xFFFF,
};

/* === Some default texture values === */

constexpr uint8_t WHITE[] = {
    255, 255, 255, 255
};

} // namespace

/* === Public Implementation === */

namespace render {

AssetCache::AssetCache()
    : mTextureWhite(
        { .pixels = (void*)WHITE, .w = 1, .h = 1, .format = HP_PIXEL_FORMAT_RGB8 },
        HP_TEXTURE_FILTER_POINT, HP_TEXTURE_WRAP_CLAMP, 1.0f
    )
    , mFont(
        FONT_TTF, FONT_TTF_SIZE, HP_FONT_SDF, 32,
        CODEPOINTS, SDL_arraysize(CODEPOINTS)
    )
{ }

const gpu::Texture& AssetCache::textureSsaoKernel()
{
    if (mTextureSsaoKernel.isValid()) {
        return mTextureSsaoKernel;
    }

    constexpr int size = 32;
    uint16_t kernel[3 * size];
    for (int i = 0; i < size; i++)
    {
        HP_Vec3 sample;
        sample.x = HP_RandRangeFloat(NULL, -1.0f, 1.0f);
        sample.y = HP_RandRangeFloat(NULL, -1.0f, 1.0f);
        sample.z = HP_RandFloat(NULL);

        sample = HP_Vec3Normalize(sample) * HP_RandFloat(NULL);
        sample *= HP_Lerp(0.1f, 1.0f, HP_POW2(static_cast<float>(i) / size));

        kernel[i * 3 + 0] = fp16_ieee_from_fp32_value(sample.x);
        kernel[i * 3 + 1] = fp16_ieee_from_fp32_value(sample.y);
        kernel[i * 3 + 2] = fp16_ieee_from_fp32_value(sample.z);
    }

    mTextureSsaoKernel = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGB16F,
            .data = kernel,
            .width = size,
            .height = 1,
            .depth = 0,
            .mipmap = false
        },
        gpu::TextureParam
        {
            .minFilter = GL_NEAREST,
            .magFilter = GL_NEAREST,
            .sWrap = GL_REPEAT,
            .tWrap = GL_REPEAT
        }
    );

    return mTextureSsaoKernel;
}

const gpu::Texture& AssetCache::textureSsaoNoise()
{
    if (mTextureSsaoNoise.isValid()) {
        return mTextureSsaoNoise;
    }

    constexpr int size = 4;
    uint16_t noise[2 * size * size];
    for (int i = 0; i < size * size; i++) {
        noise[i * 2 + 0] = fp16_ieee_from_fp32_value(HP_RandRangeFloat(NULL, -1.0f, 1.0f));
        noise[i * 2 + 1] = fp16_ieee_from_fp32_value(HP_RandRangeFloat(NULL, -1.0f, 1.0f));
    }

    mTextureSsaoNoise = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RG16F,
            .data = noise,
            .width = size,
            .height = size
        },
        gpu::TextureParam
        {
            .minFilter = GL_NEAREST,
            .magFilter = GL_NEAREST,
            .sWrap = GL_REPEAT,
            .tWrap = GL_REPEAT
        }
    );

    return mTextureSsaoNoise;
}

const gpu::Texture& AssetCache::textureBrdfLut()
{
    if (mTextureBrdfLut.isValid()) {
        return mTextureBrdfLut;
    }

    mTextureBrdfLut = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RG16F,
            .data = BRDF_LUT_512_RG16_FLOAT_RAW,
            .width = 512,
            .height = 512,
            .depth = 0,
            .mipmap = false,
        },
        gpu::TextureParam
        {
            .minFilter = GL_NEAREST,
            .magFilter = GL_NEAREST,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE
        }
    );

    return mTextureBrdfLut;
}

const gpu::Texture& AssetCache::textureNormal()
{
    if (mTextureNormal.isValid()) {
        return mTextureNormal;
    }

    constexpr uint8_t normal[] = {
        128, 128, 255
    };

    mTextureNormal = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGB8,
            .data = normal,
            .width = 1,
            .height = 1,
            .depth = 0,
            .mipmap = false
        },
        gpu::TextureParam
        {
            .minFilter = GL_NEAREST,
            .magFilter = GL_NEAREST,
            .sWrap = GL_REPEAT,
            .tWrap = GL_REPEAT
        }
    );

    return mTextureNormal;
}

} // namespace render
