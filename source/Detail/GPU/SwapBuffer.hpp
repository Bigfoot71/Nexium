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
    SwapBuffer(GLenum internalFormat, int w, int h) noexcept;

    SwapBuffer(const SwapBuffer&) = delete;
    SwapBuffer& operator=(const SwapBuffer&) = delete;
    SwapBuffer(SwapBuffer&& other) noexcept;
    SwapBuffer& operator=(SwapBuffer&& other) noexcept;

    const Framebuffer& target() const noexcept;
    const Texture& source() const noexcept;
    void swap() noexcept;

private:
    struct Buffer {
        Framebuffer framebuffer{};
        Texture texture{};
    };

private:
    std::array<Buffer, 2> mBuffers{};
    int mTargetIdx{};
};

/* === Public Implementation === */

inline SwapBuffer::SwapBuffer(GLenum internalFormat, int w, int h) noexcept
{
    for (int i = 0; i < 2; i++) {
        mBuffers[i].texture = gpu::Texture(
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
        mBuffers[i].framebuffer = gpu::Framebuffer(
            {&mBuffers[i].texture}
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
    return mBuffers[mTargetIdx].framebuffer;
}

inline const Texture& SwapBuffer::source() const noexcept
{
    return mBuffers[!mTargetIdx].texture;
}

inline void SwapBuffer::swap() noexcept
{
    mTargetIdx = !mTargetIdx;
}

} // namespace gpu

#endif // HP_GPU_SWAP_BUFFER_HPP
