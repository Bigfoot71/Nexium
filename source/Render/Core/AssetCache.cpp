/* AssetCache.cpp -- Manage internal asset storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./AssetCache.hpp"

#include <NX/NX_Rand.h>
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
// - otfinfo -u font.ttf | awk '{printf "0x%s,", substr($1,4); if (NR % 8 == 0) printf "\n"} END {if (NR % 8 != 0) printf "\n"}'
constexpr int CODEPOINTS[] = {
    0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
    0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
    0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
    0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
    0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
    0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
    0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
    0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
    0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
    0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
    0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
    0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x00A0,
    0x00A1,0x00A2,0x00A3,0x00A5,0x00A6,0x00A8,0x00A9,0x00AB,
    0x00AC,0x00AE,0x00B0,0x00B1,0x00B4,0x00B5,0x00B6,0x00B7,
    0x00B8,0x00BB,0x00BF,0x00C0,0x00C1,0x00C2,0x00C3,0x00C4,
    0x00C5,0x00C6,0x00C7,0x00C8,0x00C9,0x00CA,0x00CB,0x00CC,
    0x00CD,0x00CE,0x00CF,0x00D0,0x00D1,0x00D2,0x00D3,0x00D4,
    0x00D5,0x00D6,0x00D7,0x00D8,0x00D9,0x00DA,0x00DB,0x00DC,
    0x00DD,0x00DE,0x00DF,0x00E0,0x00E1,0x00E2,0x00E3,0x00E4,
    0x00E5,0x00E6,0x00E7,0x00E8,0x00E9,0x00EA,0x00EB,0x00EC,
    0x00ED,0x00EE,0x00EF,0x00F0,0x00F1,0x00F2,0x00F3,0x00F4,
    0x00F5,0x00F6,0x00F7,0x00F8,0x00F9,0x00FA,0x00FB,0x00FC,
    0x00FD,0x00FE,0x00FF,0x0108,0x0109,0x010C,0x010D,0x010E,
    0x010F,0x011A,0x011B,0x011C,0x011D,0x0124,0x0125,0x0131,
    0x0134,0x0135,0x0147,0x0148,0x0152,0x0153,0x0158,0x0159,
    0x015C,0x015D,0x0160,0x0161,0x0164,0x0165,0x016C,0x016D,
    0x016E,0x016F,0x0178,0x017D,0x017E,0x02C6,0x02C7,0x02D8,
    0x02DA,0x02DC,0x2013,0x2014,0x2018,0x2019,0x201A,0x201C,
    0x201D,0x201E,0x2020,0x2021,0x2022,0x2026,0x2030,0x2039,
    0x203A,0x20AC,0x20B1,0x20B7,0x2117,0x2122,0xFFFF,
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
        { .pixels = (void*)WHITE, .w = 1, .h = 1, .format = NX_PIXEL_FORMAT_RGB8 },
        NX_TEXTURE_FILTER_POINT, NX_TEXTURE_WRAP_CLAMP, 1.0f
    )
    , mFont(
        FONT_TTF, FONT_TTF_SIZE, NX_FONT_MONO, 16,
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
        NX_Vec3 sample;
        sample.x = NX_RandRangeFloat(NULL, -1.0f, 1.0f);
        sample.y = NX_RandRangeFloat(NULL, -1.0f, 1.0f);
        sample.z = NX_RandFloat(NULL);

        sample = NX_Vec3Normalize(sample) * NX_RandFloat(NULL);
        sample *= NX_Lerp(0.1f, 1.0f, NX_POW2(static_cast<float>(i) / size));

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
        noise[i * 2 + 0] = fp16_ieee_from_fp32_value(NX_RandRangeFloat(NULL, -1.0f, 1.0f));
        noise[i * 2 + 1] = fp16_ieee_from_fp32_value(NX_RandRangeFloat(NULL, -1.0f, 1.0f));
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
