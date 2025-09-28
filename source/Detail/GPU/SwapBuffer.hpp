/* SwapBuffer.hpp -- High-level management class of ping-pong buffer
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_GPU_SWAP_BUFFER_HPP
#define HP_GPU_SWAP_BUFFER_HPP

#include "./Framebuffer.hpp"
#include "./Texture.hpp"
#include <array>

namespace gpu {

/* === Declaration === */

class SwapBuffer {
public:
    SwapBuffer() = default;
    SwapBuffer(int w, int h, GLenum internalFormat) noexcept;

    SwapBuffer(const SwapBuffer&) = delete;
    SwapBuffer& operator=(const SwapBuffer&) = delete;
    SwapBuffer(SwapBuffer&& other) noexcept;
    SwapBuffer& operator=(SwapBuffer&& other) noexcept;

    const Framebuffer& target() const noexcept;
    const Texture& source() const noexcept;
    void swap() noexcept;

private:
    using Buffer = std::pair<Framebuffer, Texture>;
    std::array<Buffer, 2> mBuffers{};
    int mTargetIdx{};
};

/* === Public Implementation === */

inline SwapBuffer::SwapBuffer(int w, int h, GLenum internalFormat) noexcept
{
    for (int i = 0; i < 2; i++) {
        mBuffers[i].second = gpu::Texture(
            TextureConfig
            {
                .target = GL_TEXTURE_2D,
                .internalFormat = internalFormat,
                .data = nullptr,
                .width = w,
                .height = h
            },
            TextureParam
            {
                .minFilter = GL_LINEAR,
                .magFilter = GL_LINEAR,
                .sWrap = GL_CLAMP_TO_EDGE,
                .tWrap = GL_CLAMP_TO_EDGE,
                .rWrap = GL_CLAMP_TO_EDGE
            }
        );
        mBuffers[i].first = gpu::Framebuffer(
            {&mBuffers[i].second}
        );
    }
}

inline SwapBuffer::SwapBuffer(SwapBuffer&& other) noexcept
    : mBuffers(std::move(other.mBuffers))
    , mTargetIdx(other.mTargetIdx)
{ }

inline SwapBuffer& SwapBuffer::operator=(SwapBuffer&& other) noexcept
{
    if (this != &other) {
        mBuffers = std::move(other.mBuffers);
        mTargetIdx = other.mTargetIdx;
    }
    return *this;
}

inline const Framebuffer& SwapBuffer::target() const noexcept
{
    return mBuffers[mTargetIdx].first;
}

inline const Texture& SwapBuffer::source() const noexcept
{
    return mBuffers[!mTargetIdx].second;
}

inline void SwapBuffer::swap() noexcept
{
    mTargetIdx = !mTargetIdx;
}

} // namespace gpu

#endif // HP_GPU_SWAP_BUFFER_HPP
