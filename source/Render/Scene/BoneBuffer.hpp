/* BoneBuffer.hpp -- Bone calculation and upload management class for GPU skinning
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_BONE_BUFFER_HPP
#define NX_SCENE_BONE_BUFFER_HPP

#include "../../Detail/Util/DynamicArray.hpp"
#include "../../Detail/GPU/Buffer.hpp"
#include <NX/NX_Math.h>

namespace scene {

/* === Declaration === */

class BoneBuffer {
public:
    BoneBuffer();
    int upload(const NX_Mat4* offsets, const NX_Mat4* matrices, int count);
    const gpu::Buffer& buffer() const;
    void clear();

private:
    util::DynamicArray<NX_Mat4> mTemp{};
    gpu::Buffer mBuffer{};
    int mCurrentOffset{};
};

/* === Public Implementation === */

inline BoneBuffer::BoneBuffer()
    : mBuffer(GL_SHADER_STORAGE_BUFFER, 1024 * sizeof(NX_Mat4), nullptr, GL_DYNAMIC_DRAW)
{
    if (!mTemp.reserve(256)) {
        NX_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the bone matrix computing buffer");
    }
}

inline int BoneBuffer::upload(const NX_Mat4* offsets, const NX_Mat4* matrices, int count)
{
    /* --- Compute matrices --- */

    mTemp.clear();
    mTemp.resize(count);
    NX_Mat4MulBatch(mTemp.data(), offsets, matrices, count);

    /* --- Upload matrices --- */

    size_t byteOffset = (size_t)mCurrentOffset * sizeof(NX_Mat4);
    mBuffer.reserve((mCurrentOffset + count) * sizeof(NX_Mat4), true);
    mBuffer.upload(byteOffset, sizeof(NX_Mat4) * count, mTemp.data());

    /* --- Update and return offset --- */

    int offset = mCurrentOffset;
    mCurrentOffset += count;
    return offset;
}

inline const gpu::Buffer& BoneBuffer::buffer() const
{
    return mBuffer;
}

inline void BoneBuffer::clear()
{
    mCurrentOffset = 0;
}

} // namespace scene

#endif // NX_SCENE_BONE_BUFFER_HPP
