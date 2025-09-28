/* BoneBufferManager.hpp -- Bone calculation and upload management class for GPU skinning
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_BONE_MATRIX_CACHE_HPP
#define HP_SCENE_BONE_MATRIX_CACHE_HPP

#include "../../Detail/Util/DynamicArray.hpp"
#include "../../Detail/GPU/Buffer.hpp"
#include "Hyperion/HP_Math.h"
#include <array>

namespace scene {

/* === Declaration === */

class BoneBufferManager {
public:
    BoneBufferManager();

    int upload(const HP_Mat4* offsets, const HP_Mat4* matrices, int count);
    const gpu::Buffer& buffer() const;
    void clear();

private:
    std::array<gpu::Buffer, 3> mBuffers{};
    util::DynamicArray<HP_Mat4> mTemp{};
    int mCurrentOffset{};
    int mBufferIndex{};
};

/* === Public Implementation === */

inline BoneBufferManager::BoneBufferManager()
{
    for (auto& buffer : mBuffers) {
        buffer = gpu::Buffer(GL_SHADER_STORAGE_BUFFER, 1024 * sizeof(HP_Mat4), nullptr, GL_DYNAMIC_DRAW);
    }

    if (!mTemp.reserve(256)) {
        HP_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the bone matrix computing buffer");
    }
}

inline int BoneBufferManager::upload(const HP_Mat4* offsets, const HP_Mat4* matrices, int count)
{
    /* --- Compute matrices --- */

    mTemp.clear();
    mTemp.resize(count);
    HP_Mat4MulBatch(mTemp.data(), offsets, matrices, count);

    /* --- Upload matrices --- */

    gpu::Buffer& buffer = mBuffers[mBufferIndex];
    size_t byteOffset = (size_t)mCurrentOffset * sizeof(HP_Mat4);
    buffer.reserve((mCurrentOffset + count) * sizeof(HP_Mat4), true);
    buffer.upload(byteOffset, sizeof(HP_Mat4) * count, mTemp.data());

    /* --- Update and return offset --- */

    int offset = mCurrentOffset;
    mCurrentOffset += count;
    return offset;
}

inline const gpu::Buffer& BoneBufferManager::buffer() const
{
    return mBuffers[mBufferIndex];
}

inline void BoneBufferManager::clear()
{
    mBufferIndex = (mBufferIndex + 1) % mBuffers.size();
    mCurrentOffset = 0;
}

} // namespace scene

#endif // HP_SCENE_BONE_MATRIX_CACHE_HPP
