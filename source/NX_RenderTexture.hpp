/* NX_RenderTexture.hpp -- API definitions for Nexium's render texture module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_TEXTURE_HPP
#define NX_RENDER_TEXTURE_HPP

#include <NX/NX_RenderTexture.h>

#include "./Detail/GPU/Framebuffer.hpp"
#include "./Detail/GPU/Texture.hpp"
#include "./NX_Texture.hpp"

struct NX_RenderTexture {
    gpu::Framebuffer gpu;
    gpu::Texture depth;
    NX_Texture* color;
    ~NX_RenderTexture();
};

#endif // NX_RENDER_TEXTURE_HPP
