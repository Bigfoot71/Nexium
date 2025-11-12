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
#include "./Buffer.hpp"

namespace gpu {

/* === Declaration === */

template <typename T>
class StagingBuffer {
public:
    /** Constructors */
    StagingBuffer() noexcept = default;
    StagingBuffer(GLenum target, int initialCapacity) noexcept;

    /** Casts */
    operator const gpu::Buffer&() const noexcept;

    /** Update methods */
    T* StageMap(int count, int* index) noexcept;
    int Stage(const T& data) noexcept;
    void Upload() noexcept;

    /** Getters */
    const gpu::Buffer& GetBuffer() const noexcept;

private:
    util::DynamicArray<T> mStagingBuffer{};
    gpu::Buffer mBuffer{};
};

/* === Public Implementation === */

template <typename T>
StagingBuffer<T>::StagingBuffer(GLenum target, int initialCapacity) noexcept
    : mBuffer(target, initialCapacity * sizeof(T), nullptr, GL_STATIC_DRAW)
{
    if (!mStagingBuffer.Reserve(initialCapacity)) {
        NX_LOG(E, "RENDER: Staging buffer memory reservation failed (requested: %i entries)", initialCapacity);
    }
}

template <typename T>
StagingBuffer<T>::operator const gpu::Buffer&() const noexcept
{
    return mBuffer;
}

template <typename T>
T* StagingBuffer<T>::StageMap(int count, int* index) noexcept
{
    SDL_assert(index != nullptr);
    *index = static_cast<int>(mStagingBuffer.GetSize());
    mStagingBuffer.Resize(*index + count);
    return &mStagingBuffer[*index];
}

template <typename T>
int StagingBuffer<T>::Stage(const T& data) noexcept
{
    int index = static_cast<int>(mStagingBuffer.GetSize());
    mStagingBuffer.PushBack(data);
    return index;
}

template <typename T>
void StagingBuffer<T>::Upload() noexcept
{
    if (mStagingBuffer.IsEmpty()) {
        return;
    }

    size_t size = mStagingBuffer.GetSize() * sizeof(T);

    mBuffer.Reserve(size, false);
    mBuffer.Upload(0, size, mStagingBuffer.GetData());

    mStagingBuffer.Clear();
}

template <typename T>
const gpu::Buffer& StagingBuffer<T>::GetBuffer() const noexcept
{
    return mBuffer;
}

} // namespace gpu

#endif // NX_GPU_STAGING_BUFFER_HPP
