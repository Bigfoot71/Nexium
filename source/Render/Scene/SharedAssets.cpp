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

#include "./SharedAssets.hpp"
#include <Hyperion/HP_Rand.h>
#include <SDL3/SDL_stdinc.h>
#include <fp16.h>

#include <assets/brdf_lut_512_rg16_float.raw.h>

/* === SharedAssets === */

namespace scene {

const gpu::Texture& SharedAssets::textureBrdfLut()
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

const gpu::Texture& SharedAssets::textureNormal()
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

const gpu::Texture& SharedAssets::textureSsaoKernel()
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

const gpu::Texture& SharedAssets::textureSsaoNoise()
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

} // namespace scene
