/* NX_RenderTexture.hpp -- Implementation of the API for render textures
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_TEXTURE_HPP
#define NX_RENDER_TEXTURE_HPP

#include "../Detail/GPU/Framebuffer.hpp"
#include "../Detail/GPU/Pipeline.hpp"
#include "../Detail/GPU/Texture.hpp"
#include "./NX_Texture.hpp"

/* === Declaration === */

class NX_RenderTexture {
public:
    NX_RenderTexture(int w, int h);

    bool isValid() const;
    NX_Texture& texture();
    const NX_Texture& texture() const;
    const gpu::Framebuffer& framebuffer() const;
    void blit(int xDst, int yDst, int wDst, int hDst, bool linear) const;

private:
    gpu::Framebuffer mFramebuffer;
    gpu::Texture mDepthTarget;
    NX_Texture mColorTarget;
};

/* === Public Implementation === */

inline NX_RenderTexture::NX_RenderTexture(int w, int h)
    : mColorTarget(w, h)
{
    mDepthTarget = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_DEPTH_COMPONENT24,
            .data = nullptr,
            .width = w,
            .height = h,
        }
    );

    mFramebuffer = gpu::Framebuffer(
        {&mColorTarget.gpuTexture()},
        &mDepthTarget
    );
}

inline bool NX_RenderTexture::isValid() const
{
    return mFramebuffer.isValid();
}

inline NX_Texture& NX_RenderTexture::texture()
{
    return mColorTarget;
}

inline const NX_Texture& NX_RenderTexture::texture() const
{
    return mColorTarget;
}

inline const gpu::Framebuffer& NX_RenderTexture::framebuffer() const
{
    return mFramebuffer;
}

inline void NX_RenderTexture::blit(int xDst, int yDst, int wDst, int hDst, bool linear) const
{
    gpu::Pipeline::blitToBackBuffer(mFramebuffer, xDst, yDst, wDst, hDst, linear);
}

#endif // NX_RENDER_TEXTURE_HPP
