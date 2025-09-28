/* HP_RenderTexture.hpp -- Implementation of the API for render textures
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_RENDER_TEXTURE_HPP
#define HP_RENDER_TEXTURE_HPP

#include "../Detail/GPU/Framebuffer.hpp"
#include "../Detail/GPU/Pipeline.hpp"
#include "../Detail/GPU/Texture.hpp"
#include "./HP_Texture.hpp"

/* === Declaration === */

class HP_RenderTexture {
public:
    HP_RenderTexture(int w, int h);

    bool isValid() const;
    HP_Texture& texture();
    const HP_Texture& texture() const;
    const gpu::Framebuffer& framebuffer() const;
    void blit(int xDst, int yDst, int wDst, int hDst, bool linear) const;

private:
    gpu::Framebuffer mFramebuffer;
    gpu::Texture mDepthTarget;
    HP_Texture mColorTarget;
};

/* === Public Implementation === */

inline HP_RenderTexture::HP_RenderTexture(int w, int h)
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

inline bool HP_RenderTexture::isValid() const
{
    return mFramebuffer.isValid();
}

inline HP_Texture& HP_RenderTexture::texture()
{
    return mColorTarget;
}

inline const HP_Texture& HP_RenderTexture::texture() const
{
    return mColorTarget;
}

inline const gpu::Framebuffer& HP_RenderTexture::framebuffer() const
{
    return mFramebuffer;
}

inline void HP_RenderTexture::blit(int xDst, int yDst, int wDst, int hDst, bool linear) const
{
    gpu::Pipeline::blitToBackBuffer(mFramebuffer, xDst, yDst, wDst, hDst, linear);
}

#endif // HP_RENDER_TEXTURE_HPP
