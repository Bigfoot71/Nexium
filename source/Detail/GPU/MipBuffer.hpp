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
    int GetNumLevels() const noexcept;
    int GetWidth(int level) const noexcept;
    int GetHeight(int level) const noexcept;
    NX_IVec2 GetDimensions(int level) const noexcept;
    const gpu::Texture& GetLevel(int level) noexcept;
    const gpu::Texture& GetTexture() const noexcept;

    template <std::invocable<> Func>
    void RenderLevel(const gpu::Pipeline& pipeline, int level, Func&& f) noexcept;

    template <std::invocable<int, int> Func>
    void Downsample(const gpu::Pipeline& pipeline, int firstLevel, Func&& f) noexcept;

    template <std::invocable<int> Func>
    void Iterate(const gpu::Pipeline& pipeline, Func&& f) noexcept;

    template <std::invocable<int, int> Func>
    void Upsample(const gpu::Pipeline& pipeline, Func&& func) noexcept;

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
            .mipmap = true,
            .immutable = true
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

inline int MipBuffer::GetNumLevels() const noexcept
{
    return mTexture.GetNumLevels();
}

inline int MipBuffer::GetWidth(int level) const noexcept
{
    return NX_MAX(1, mTexture.GetWidth() >> level);
}

inline int MipBuffer::GetHeight(int level) const noexcept
{
    return NX_MAX(1, mTexture.GetHeight() >> level);
}

inline NX_IVec2 MipBuffer::GetDimensions(int level) const noexcept
{
    int w = NX_MAX(1, mTexture.GetWidth() >> level);
    int h = NX_MAX(1, mTexture.GetHeight() >> level);

    return NX_IVEC2(w, h);
}

inline const gpu::Texture& MipBuffer::GetLevel(int level) noexcept
{
    mTexture.SetMipLevelRange(level, level);
    return mTexture;
}

inline const gpu::Texture& MipBuffer::GetTexture() const noexcept
{
    return mTexture;
}

template <std::invocable<> Func>
inline void MipBuffer::RenderLevel(const gpu::Pipeline& pipeline, int level, Func&& f) noexcept
{
    mFramebuffer.SetColorAttachmentTarget(0, 0, 0, level);
    pipeline.BindFramebuffer(mFramebuffer);
    pipeline.SetViewport(GetDimensions(level));

    f();
}

template <std::invocable<int, int> Func>
void MipBuffer::Downsample(const gpu::Pipeline& pipeline, int firstLevel, Func&& f) noexcept
{
    pipeline.BindFramebuffer(mFramebuffer);

    for (int dstLevel = firstLevel, srcLevel = firstLevel - 1; dstLevel < mTexture.GetNumLevels(); dstLevel++, srcLevel++) {
        mFramebuffer.SetColorAttachmentTarget(0, 0, 0, dstLevel);
        pipeline.SetViewport(GetDimensions(dstLevel));
        if (srcLevel >= 0) {
            mTexture.SetMipLevelRange(srcLevel, srcLevel);
        }
        f(dstLevel, srcLevel);
    }

    // Reset mipmap sampling levels for debugging with RenderDoc
    if constexpr (INX_BuildInfo::debug) {
        mTexture.SetMipLevelRange(0, mTexture.GetNumLevels() - 1);
    }
}

template <std::invocable<int> Func>
void MipBuffer::Iterate(const gpu::Pipeline& pipeline, Func&& f) noexcept
{
    pipeline.BindFramebuffer(mFramebuffer);

    for (int dstLevel = 0; dstLevel < mTexture.GetNumLevels(); dstLevel++) {
        mFramebuffer.SetColorAttachmentTarget(0, 0, 0, dstLevel);
        pipeline.SetViewport(GetDimensions(dstLevel));
        f(dstLevel);
    }
}

template <std::invocable<int, int> Func>
void MipBuffer::Upsample(const gpu::Pipeline& pipeline, Func&& f) noexcept
{
    pipeline.BindFramebuffer(mFramebuffer);

    for (int srcLevel = mTexture.GetNumLevels() - 1, dstLevel = srcLevel - 1; srcLevel > 0; srcLevel--, dstLevel--) {
        mFramebuffer.SetColorAttachmentTarget(0, 0, 0, dstLevel);
        mTexture.SetMipLevelRange(srcLevel, srcLevel);
        pipeline.SetViewport(GetDimensions(dstLevel));
        f(dstLevel, srcLevel);
    }

    // Reset mipmap sampling levels for debugging with RenderDoc
    if constexpr (INX_BuildInfo::debug) {
        mTexture.SetMipLevelRange(0, mTexture.GetNumLevels() - 1);
    }
}

} // namespace gpu

#endif // NX_GPU_MIP_BUFFER_HPP
