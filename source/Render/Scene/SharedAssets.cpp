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
#include "SDL3/SDL_stdinc.h"

#include <assets/brdf_lut_512_rg16_float.raw.h>

namespace {
constexpr uint8_t NORMAL[] = { 128, 128, 255 };
}

/* === SharedAssets === */

namespace scene {

SharedAssets::SharedAssets()
    : mTextureBrdfLut(
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
    )
    , mTextureNormal(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGB8,
            .data = NORMAL,
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
    )
{ }

} // namespace scene
