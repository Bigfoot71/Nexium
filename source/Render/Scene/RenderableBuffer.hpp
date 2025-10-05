/* RenderableBuffer.hpp -- GPU renderable data upload management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_RENDERABLE_BUFFER_HPP
#define HP_SCENE_RENDERABLE_BUFFER_HPP

#include <Hyperion/HP_Core.h>
#include <Hyperion/HP_Math.h>

#include "./DrawData.hpp"
#include "./DrawCall.hpp"

namespace scene {

/* === Declaration === */

class RenderableBuffer {
public:
    RenderableBuffer();

    void upload(const DrawData& data, const DrawCall& call);
    const gpu::Buffer& buffer() const;

private:
    struct GPUData {
        alignas(16) HP_Mat4 matModel;
        alignas(16) HP_Mat4 matNormal;
        alignas(4) int32_t boneOffset;
        alignas(4) uint32_t layerMask;
        alignas(4) int32_t instancing;
        alignas(4) int32_t skinning;
    };

private:
    std::array<gpu::Buffer, 3> mBuffers{};
    int mBufferIndex{};
};

/* === Public Implementaiton === */

inline RenderableBuffer::RenderableBuffer()
{
    for (auto& buffer : mBuffers) {
        buffer = gpu::Buffer(GL_UNIFORM_BUFFER, sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW);
    }
}

inline void RenderableBuffer::upload(const DrawData& data, const DrawCall& call)
{
    GPUData gpuData{
        .matModel = data.matrix(),
        .matNormal = HP_Mat3ToMat4(&data.normal()),
        .boneOffset = data.boneMatrixOffset(),
        .layerMask = call.mesh().layerMask,
        .instancing = data.useInstancing(),
        .skinning = data.useSkinning(),
    };

    mBufferIndex = (mBufferIndex + 1) % mBuffers.size();
    mBuffers[mBufferIndex].upload(&gpuData);
}

inline const gpu::Buffer& RenderableBuffer::buffer() const
{
    return mBuffers[mBufferIndex];
}

} // namespace scene

#endif // HP_SCENE_RENDERABLE_BUFFER_HPP
