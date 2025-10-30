/* Buffer.hpp -- Allows high-level staging buffer management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_STAGING_BUFFER_HPP
#define NX_GPU_STAGING_BUFFER_HPP

#include "../Util/DynamicArray.hpp"
#include "../Util/ObjectRing.hpp"
#include "./Buffer.hpp"

namespace gpu {

/* === Declaration === */

template <typename T, int BufferCount>
class StagingBuffer {
public:
    /** Constructors */
    StagingBuffer(GLenum target, int initialCapacity) noexcept;

    /** Update methods */
    T* stageMap(int count, int* index) noexcept;
    int stage(const T& data) noexcept;
    void upload() noexcept;

    /** Getters */
    const gpu::Buffer& buffer() const noexcept;

private:
    util::ObjectRing<gpu::Buffer, BufferCount> mBuffer;
    util::DynamicArray<T> mStagingBuffer{};
};

/* === Public Implementation === */

template <typename T, int BufferCount>
StagingBuffer<T, BufferCount>::StagingBuffer(GLenum target, int initialCapacity) noexcept
    : mBuffer(target, initialCapacity * sizeof(T), nullptr, GL_STATIC_DRAW)
{
    if (!mStagingBuffer.reserve(initialCapacity)) {
        NX_LOG(E, "RENDER: Staging buffer memory reservation failed (requested: %i entries)", initialCapacity);
    }
}

template <typename T, int BufferCount>
T* StagingBuffer<T, BufferCount>::stageMap(int count, int* index) noexcept
{
    SDL_assert(index != nullptr);

    *index = static_cast<int>(mStagingBuffer.size());

    if constexpr (BufferCount > 1) {
        if (index == 0) mBuffer.rotate();
    }

    mStagingBuffer.resize(*index + count);

    return &mStagingBuffer[*index];
}

template <typename T, int BufferCount>
int StagingBuffer<T, BufferCount>::stage(const T& data) noexcept
{
    int index = static_cast<int>(mStagingBuffer.size());

    if constexpr (BufferCount > 1) {
        if (index == 0) mBuffer.rotate();
    }

    mStagingBuffer.push_back(data);

    return index;
}

template <typename T, int BufferCount>
void StagingBuffer<T, BufferCount>::upload() noexcept
{
    if (mStagingBuffer.empty()) {
        return;
    }

    size_t size = mStagingBuffer.size() * sizeof(T);

    mBuffer->reserve(size, false);
    mBuffer->upload(0, size, mStagingBuffer.data());

    mStagingBuffer.clear();
}

template <typename T, int BufferCount>
const gpu::Buffer& StagingBuffer<T, BufferCount>::buffer() const noexcept
{
    return *mBuffer;
}

} // namespace gpu

#endif // NX_GPU_STAGING_BUFFER_HPP
