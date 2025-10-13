/* MipBuffer.hpp -- High-level management class of mip chain
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_MIP_BUFFER_HPP
#define NX_GPU_MIP_BUFFER_HPP

#include "./Framebuffer.hpp"
#include "./Pipeline.hpp"
#include "./Texture.hpp"
#include <concepts>

namespace gpu {

/* === Declaration === */

class MipBuffer {
public:
    /** Constructors */
    MipBuffer() = default;
    MipBuffer(int w, int h, GLenum internalFormat) noexcept;

    /** Move only */
    MipBuffer(const MipBuffer&) = delete;
    MipBuffer& operator=(const MipBuffer&) = delete;
    MipBuffer(MipBuffer&& other) noexcept;
    MipBuffer& operator=(MipBuffer&& other) noexcept;

    /** Getters */
    int numLevels() const noexcept;
    int width(int level) const noexcept;
    int height(int level) const noexcept;
    NX_IVec2 dimensions(int level) const noexcept;
    const gpu::Texture& texture() const noexcept;

    template <std::invocable<int, int> Func>
    void downsample(const gpu::Pipeline& pipeline, int firstLevel, Func&& f) noexcept;

    template <std::invocable<int> Func>
    void iterate(const gpu::Pipeline& pipeline, Func&& f) noexcept;

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

inline int MipBuffer::numLevels() const noexcept
{
    return mTexture.numLevels();
}

inline int MipBuffer::width(int level) const noexcept
{
    return NX_MAX(1, mTexture.width() >> level);
}

inline int MipBuffer::height(int level) const noexcept
{
    return NX_MAX(1, mTexture.height() >> level);
}

inline NX_IVec2 MipBuffer::dimensions(int level) const noexcept
{
    int w = NX_MAX(1, mTexture.width() >> level);
    int h = NX_MAX(1, mTexture.height() >> level);

    return NX_IVEC2(w, h);
}

inline const gpu::Texture& MipBuffer::texture() const noexcept
{
    return mTexture;
}

template <std::invocable<int, int> Func>
void MipBuffer::downsample(const gpu::Pipeline& pipeline, int firstLevel, Func&& f) noexcept
{
    pipeline.bindFramebuffer(mFramebuffer);

    for (int dstLevel = firstLevel, srcLevel = firstLevel - 1; dstLevel < mTexture.numLevels(); dstLevel++, srcLevel++) {
        mFramebuffer.setColorAttachmentTarget(0, 0, 0, dstLevel);
        pipeline.setViewport(dimensions(dstLevel));
        if (srcLevel >= 0) {
            mTexture.setMipLevelRange(srcLevel, srcLevel);
        }
        f(dstLevel, srcLevel);
    }

    // Reset mipmap sampling levels for debugging with RenderDoc
    if constexpr (detail::BuildInfo::debug) {
        mTexture.setMipLevelRange(0, mTexture.numLevels() - 1);
    }
}

template <std::invocable<int> Func>
void MipBuffer::iterate(const gpu::Pipeline& pipeline, Func&& f) noexcept
{
    pipeline.bindFramebuffer(mFramebuffer);

    for (int dstLevel = 0; dstLevel < mTexture.numLevels(); dstLevel++) {
        mFramebuffer.setColorAttachmentTarget(0, 0, 0, dstLevel);
        pipeline.setViewport(dimensions(dstLevel));
        f(dstLevel);
    }
}

template <std::invocable<int, int> Func>
void MipBuffer::upsample(const gpu::Pipeline& pipeline, Func&& f) noexcept
{
    pipeline.bindFramebuffer(mFramebuffer);

    for (int srcLevel = mTexture.numLevels() - 1, dstLevel = srcLevel - 1; srcLevel > 0; srcLevel--, dstLevel--) {
        mFramebuffer.setColorAttachmentTarget(0, 0, 0, dstLevel);
        mTexture.setMipLevelRange(srcLevel, srcLevel);
        pipeline.setViewport(dimensions(dstLevel));
        f(dstLevel, srcLevel);
    }

    // Reset mipmap sampling levels for debugging with RenderDoc
    if constexpr (detail::BuildInfo::debug) {
        mTexture.setMipLevelRange(0, mTexture.numLevels() - 1);
    }
}

} // namespace gpu

#endif // NX_GPU_MIP_BUFFER_HPP
