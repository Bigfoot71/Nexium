/* MaterialBuffer.hpp -- GPU material upload management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_MATERIAL_BUFFER_HPP
#define NX_SCENE_MATERIAL_BUFFER_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Math.h>

#include "../../Detail/Util/DynamicArray.hpp"
#include "../../Detail/GPU/Buffer.hpp"

namespace scene {

/* === Declaration === */

class MaterialBuffer {
public:
    MaterialBuffer();

    /** Stage material data and return GPU material index */
    int stage(const NX_Material& material);

    /** Upload all staged data */
    void upload();

    /** Getters */
    const gpu::Buffer& buffer() const;

private:
    struct GPUData {
        alignas(16) NX_Vec4 albedoColor;
        alignas(16) NX_Vec3 emissionColor;
        alignas(4) float emissionEnergy;
        alignas(4) float aoLightAffect;
        alignas(4) float occlusion;
        alignas(4) float roughness;
        alignas(4) float metalness;
        alignas(4) float normalScale;
        alignas(4) float alphaCutOff;
        alignas(4) NX_Vec2 texOffset;
        alignas(4) NX_Vec2 texScale;
        alignas(4) int billboard;
    };

private:
    util::DynamicArray<GPUData> mStagingBuffer{};
    gpu::Buffer mBuffer{};
};

/* === Public Implementation === */

inline MaterialBuffer::MaterialBuffer()
    : mBuffer(GL_SHADER_STORAGE_BUFFER, 1024 * sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW)
{
    if (!mStagingBuffer.reserve(1024)) {
        NX_INTERNAL_LOG(E,
            "RENDER: Material staging buffer memory reservation failed (requested: 1024 bytes). "
            "Rendering may not proceed correctly."
        );
    }
}

inline int MaterialBuffer::stage(const NX_Material& material)
{
    int index = static_cast<int>(mStagingBuffer.size());

    mStagingBuffer.emplace_back(GPUData {
        .albedoColor = NX_ColorToVec4(material.albedo.color),
        .emissionColor = NX_ColorToVec3(material.emission.color),
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
    });

    return index;
}

inline void MaterialBuffer::upload()
{
    size_t size = mStagingBuffer.size() * sizeof(GPUData);

    mBuffer.reserve(size, false);
    mBuffer.upload(0, size, mStagingBuffer.data());

    mStagingBuffer.clear();
}

inline const gpu::Buffer& MaterialBuffer::buffer() const
{
    return mBuffer;
}

} // namespace scene

#endif // NX_SCENE_MATERIAL_BUFFER_HPP
