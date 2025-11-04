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
    StagingBuffer() noexcept = default;
    StagingBuffer(GLenum target, int initialCapacity) noexcept;

    /** Update methods */
    T* StageMap(int count, int* index) noexcept;
    int Stage(const T& data) noexcept;
    void Upload() noexcept;

    /** Getters */
    const gpu::Buffer& GetBuffer() const noexcept;

private:
    util::ObjectRing<gpu::Buffer, BufferCount> mBuffer{};
    util::DynamicArray<T> mStagingBuffer{};
};

/* === Public Implementation === */

template <typename T, int BufferCount>
StagingBuffer<T, BufferCount>::StagingBuffer(GLenum target, int initialCapacity) noexcept
    : mBuffer(target, initialCapacity * sizeof(T), nullptr, GL_STATIC_DRAW)
{
    if (!mStagingBuffer.Reserve(initialCapacity)) {
        NX_LOG(E, "RENDER: Staging buffer memory reservation failed (requested: %i entries)", initialCapacity);
    }
}

template <typename T, int BufferCount>
T* StagingBuffer<T, BufferCount>::StageMap(int count, int* index) noexcept
{
    SDL_assert(index != nullptr);

    *index = static_cast<int>(mStagingBuffer.GetSize());

    if constexpr (BufferCount > 1) {
        if (index == 0) mBuffer.Rotate();
    }

    mStagingBuffer.Resize(*index + count);

    return &mStagingBuffer[*index];
}

template <typename T, int BufferCount>
int StagingBuffer<T, BufferCount>::Stage(const T& data) noexcept
{
    int index = static_cast<int>(mStagingBuffer.GetSize());

    if constexpr (BufferCount > 1) {
        if (index == 0) mBuffer.Rotate();
    }

    mStagingBuffer.PushBack(data);

    return index;
}

template <typename T, int BufferCount>
void StagingBuffer<T, BufferCount>::Upload() noexcept
{
    if (mStagingBuffer.IsEmpty()) {
        return;
    }

    size_t size = mStagingBuffer.GetSize() * sizeof(T);

    mBuffer->Reserve(size, false);
    mBuffer->Upload(0, size, mStagingBuffer.GetData());

    mStagingBuffer.Clear();
}

template <typename T, int BufferCount>
const gpu::Buffer& StagingBuffer<T, BufferCount>::GetBuffer() const noexcept
{
    return *mBuffer;
}

} // namespace gpu

#endif // NX_GPU_STAGING_BUFFER_HPP
