/* NX_RenderTexture.cpp -- API definition for Nexium's render texture module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_RenderTexture.hpp"
#include "./NX_Texture.hpp"

#include "./Detail/Util/ObjectPool.hpp"
#include "./Detail/GPU/Framebuffer.hpp"
#include "./Detail/GPU/Pipeline.hpp"
#include "./Detail/GPU/Texture.hpp"
#include "NX/NX_RenderTexture.h"

// ============================================================================
// LOCAL MANAGEMENT
// ============================================================================

using INX_RenderTexturePool = util::ObjectPool<NX_RenderTexture, 1024>;

static INX_RenderTexturePool& INX_GetPool()
{
    static INX_RenderTexturePool pool{};
    return pool;
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_RenderTexture* NX_CreateRenderTexture(int w, int h)
{
    NX_RenderTexture* target = INX_GetPool().create();

    target->color = NX_CreateTexture(w, h, nullptr, NX_PIXEL_FORMAT_RGBA8);

    target->depth = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_DEPTH_COMPONENT24,
            .data = nullptr,
            .width = w,
            .height = h,
            .depth = 0,
            .mipmap = false
        },
        gpu::TextureParam
        {
            .minFilter = GL_NEAREST,
            .magFilter = GL_NEAREST,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE,
            .anisotropy = 1.0f
        }
    );

    target->gpu = gpu::Framebuffer(
        {&target->color->gpu},
        &target->depth
    );

    return target;
}

void NX_DestroyRenderTexture(NX_RenderTexture* target)
{
    NX_DestroyTexture(target->color);
    INX_GetPool().destroy(target);
}

NX_IVec2 NX_GetRenderTextureSize(const NX_RenderTexture* target)
{
    return target->gpu.dimensions();
}

NX_Texture* NX_GetRenderTexture(const NX_RenderTexture* target)
{
    return target->color;
}

void NX_BlitRenderTexture(const NX_RenderTexture* target, int xDst, int yDst, int wDst, int hDst, bool linear)
{
    gpu::Pipeline::blitToBackBuffer(
        *reinterpret_cast<const gpu::Framebuffer*>(target),
        xDst, yDst, wDst, hDst, linear
    );
}
