/* PerModelBuffer.hpp -- GPU renderable data upload management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_PER_MODEL_BUFFER_HPP
#define NX_SCENE_PER_MODEL_BUFFER_HPP

#include <NX/NX_Core.h>
#include <NX/NX_Math.h>

#include "../../Detail/Util/DynamicArray.hpp"
#include "../../Detail/GPU/Buffer.hpp"

namespace scene {

/* === Declaration === */

class PerModelBuffer {
public:
    PerModelBuffer();

    /** Stage object data and return GPU object index */
    int stage(const NX_Transform& transform, int instanceCount = 0, int boneMatrixOffset = -1);

    /** Upload all staged data */
    void upload();

    /** Getters */
    const gpu::Buffer& buffer() const;

private:
    struct GPUData {
        alignas(16) NX_Mat4 matModel;
        alignas(16) NX_Mat4 matNormal;
        alignas(4) int32_t boneOffset;
        alignas(4) int32_t instancing;
        alignas(4) int32_t skinning;
    };

private:
    util::DynamicArray<GPUData> mStagingBuffer{};
    gpu::Buffer mBuffer{};
};

/* === Public Implementaiton === */

inline PerModelBuffer::PerModelBuffer()
    : mBuffer(GL_SHADER_STORAGE_BUFFER, 1024 * sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW)
{
    if (!mStagingBuffer.reserve(1024)) {
        NX_INTERNAL_LOG(E,
            "RENDER: Object staging buffer memory reservation failed (requested: 1024 bytes). "
            "Rendering may not proceed correctly."
        );
    }
}

inline int PerModelBuffer::stage(const NX_Transform& transform, int instanceCount, int boneMatrixOffset)
{
    int index = static_cast<int>(mStagingBuffer.size());

    NX_Mat4 matModel = NX_TransformToMat4(&transform);
    NX_Mat3 matNormal = NX_Mat3Normal(&matModel);

    mStagingBuffer.emplace_back(GPUData {
        .matModel = matModel,
        .matNormal = NX_Mat3ToMat4(&matNormal),
        .boneOffset = boneMatrixOffset,
        .instancing = (instanceCount > 0),
        .skinning = (boneMatrixOffset >= 0)
    });

    return index;
}

inline void PerModelBuffer::upload()
{
    size_t size = mStagingBuffer.size() * sizeof(GPUData);

    mBuffer.reserve(size, false);
    mBuffer.upload(0, size, mStagingBuffer.data());

    mStagingBuffer.clear();
}

inline const gpu::Buffer& PerModelBuffer::buffer() const
{
    return mBuffer;
}

} // namespace scene

#endif // NX_SCENE_PER_MODEL_BUFFER_HPP
