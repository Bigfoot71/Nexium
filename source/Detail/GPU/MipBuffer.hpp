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

#ifndef HP_GPU_MIP_BUFFER_HPP
#define HP_GPU_MIP_BUFFER_HPP

#include "./Framebuffer.hpp"
#include "./Pipeline.hpp"
#include "./Texture.hpp"
#include <concepts>

namespace gpu {

/* === Declaration === */

class MipBuffer {
public:
    MipBuffer() = default;
    MipBuffer(int w, int h, GLenum internalFormat) noexcept;

    MipBuffer(const MipBuffer&) = delete;
    MipBuffer& operator=(const MipBuffer&) = delete;
    MipBuffer(MipBuffer&& other) noexcept;
    MipBuffer& operator=(MipBuffer&& other) noexcept;

    const gpu::Texture& texture() const noexcept;

    int numLevels() const noexcept;
    int width(int level) const noexcept;
    int height(int level) const noexcept;
    HP_IVec2 dimensions(int level) const noexcept;

    void setMipLevelRange(int baseLevel, int maxLevel) noexcept;

    template <std::invocable<int, int> Func>
    void downsample(const gpu::Pipeline& pipeline, int firstLevel, Func&& f) noexcept;

    template <std::invocable<int, int> Func>
    void upsample(const gpu::Pipeline& pipeline, Func&& func) noexcept;

private:
    gpu::Framebuffer mFramebuffer{};
    gpu::Texture mTexture{};
};

/* === Public Implementation === */

inline MipBuffer::MipBuffer(int w, int h, GLenum internalFormat) noexcept
{
    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = internalFormat,
            .data = nullptr,
            .width = w,
            .height = h,
            .depth = 0,
            .mipmap = true
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    mFramebuffer = gpu::Framebuffer(
        {&mTexture}, nullptr
    );
}

inline MipBuffer::MipBuffer(MipBuffer&& other) noexcept
    : mFramebuffer(std::move(other.mFramebuffer))
    , mTexture(std::move(other.mTexture))
{ }

inline MipBuffer& MipBuffer::operator=(MipBuffer&& other) noexcept
{
    if (this != &other) {
        mFramebuffer = std::move(other.mFramebuffer);
        mTexture = std::move(other.mTexture);
    }
    return *this;
}

inline const gpu::Texture& MipBuffer::texture() const noexcept
{
    return mTexture;
}

inline int MipBuffer::numLevels() const noexcept
{
    return mTexture.numLevels();
}

inline int MipBuffer::width(int level) const noexcept
{
    return HP_MAX(1, mTexture.width() >> level);
}

inline int MipBuffer::height(int level) const noexcept
{
    return HP_MAX(1, mTexture.height() >> level);
}

inline HP_IVec2 MipBuffer::dimensions(int level) const noexcept
{
    int w = HP_MAX(1, mTexture.width() >> level);
    int h = HP_MAX(1, mTexture.height() >> level);

    return HP_IVEC2(w, h);
}

inline void MipBuffer::setMipLevelRange(int baseLevel, int maxLevel) noexcept
{
    mTexture.setMipLevelRange(baseLevel, maxLevel);
}

template <std::invocable<int, int> Func>
void MipBuffer::downsample(const gpu::Pipeline& pipeline, int firstLevel, Func&& f) noexcept
{
    pipeline.bindFramebuffer(mFramebuffer);
    for (int dstLevel = firstLevel; dstLevel < mTexture.numLevels(); dstLevel++) {
        mFramebuffer.setColorAttachmentTarget(0, 0, 0, dstLevel);
        pipeline.setViewport(dimensions(dstLevel));
        f(dstLevel, HP_MAX(dstLevel - 1, 0));
    }
}

template <std::invocable<int, int> Func>
void MipBuffer::upsample(const gpu::Pipeline& pipeline, Func&& f) noexcept
{
    pipeline.bindFramebuffer(mFramebuffer);
    for (int srcLevel = mTexture.numLevels() - 1, dstLevel = srcLevel - 1; srcLevel > 0; srcLevel--, dstLevel--) {
        mFramebuffer.setColorAttachmentTarget(0, 0, 0, dstLevel);
        pipeline.setViewport(dimensions(dstLevel));
        f(dstLevel, srcLevel);
    }
}

} // namespace gpu

#endif // HP_GPU_MIP_BUFFER_HPP
