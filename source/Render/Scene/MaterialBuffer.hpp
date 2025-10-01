/* MaterialBuffer.hpp -- GPU material upload management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_MATERIAL_BUFFER_HPP
#define HP_SCENE_MATERIAL_BUFFER_HPP

#include "../../Detail/GPU/Buffer.hpp"
#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Math.h>
#include <array>

namespace scene {

/* === Declaration === */

class MaterialBuffer {
public:
    MaterialBuffer();

    void upload(const HP_Material& material);
    const gpu::Buffer& buffer() const;

private:
    struct GPUData {
        alignas(16) HP_Vec4 albedoColor;
        alignas(16) HP_Vec3 emissionColor;
        alignas(4) float emissionEnergy;
        alignas(4) float aoLightAffect;
        alignas(4) float occlusion;
        alignas(4) float roughness;
        alignas(4) float metalness;
        alignas(4) float normalScale;
        alignas(4) float alphaCutOff;
        alignas(4) HP_Vec2 texOffset;
        alignas(4) HP_Vec2 texScale;
        alignas(4) int billboard;
    };

private:
    std::array<gpu::Buffer, 3> mBuffers{};
    int mBufferIndex{};
};

/* === Public Implementation === */

inline MaterialBuffer::MaterialBuffer()
{
    for (auto& buffer : mBuffers) {
        buffer = gpu::Buffer(GL_UNIFORM_BUFFER, sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW);
    }
}

inline void MaterialBuffer::upload(const HP_Material& material)
{
    // NOTE: We could have used a single ring buffer instead of three separate UBOs,
    //       but driver behavior, especially on mobile GPUs, is not guaranteed.
    //       Some drivers may implicitly synchronize or mark the entire buffer as busy
    //       when updating or binding a sub-range, potentially causing stalls.

    GPUData data{
        .albedoColor = HP_ColorToVec4(material.albedo.color),
        .emissionColor = HP_ColorToVec3(material.emission.color),
        .emissionEnergy = material.emission.energy,
        .aoLightAffect = material.orm.aoLightAffect,
        .occlusion = material.orm.occlusion,
        .roughness = material.orm.roughness,
        .metalness = material.orm.metalness,
        .normalScale = material.normal.scale,
        .alphaCutOff = material.alphaCutOff,
        .texOffset = material.texOffset,
        .texScale = material.texScale,
        .billboard = material.billboard
    };

    mBufferIndex = (mBufferIndex + 1) % mBuffers.size();
    mBuffers[mBufferIndex].upload(&data);
}

inline const gpu::Buffer& MaterialBuffer::buffer() const
{
    return mBuffers[mBufferIndex];
}

} // namespace scene

#endif // HP_SCENE_MATERIAL_BUFFER_HPP
