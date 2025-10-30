/* LightManager.cpp -- Direct light management class for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./LightManager.hpp"

#include "../../Detail/GPU/Pipeline.hpp"
#include "./DrawCallManager.hpp"
#include "../NX_Texture.hpp"

#include <NX/NX_Render.h>
#include <NX/NX_Macros.h>
#include <NX/NX_Math.h>
#include <cfloat>

namespace scene {

/* === Public Implementation === */

LightManager::LightManager(render::ProgramCache& programs, render::AssetCache& assets, const NX_AppDesc& desc)
    : mPrograms(programs), mAssets(assets)
    , mFrameShadowUniform(GL_UNIFORM_BUFFER, sizeof(FrameShadowUniform), nullptr, GL_DYNAMIC_DRAW)
    , mShadowResolution{desc.render3D.shadowRes > 0 ? desc.render3D.shadowRes : 2048}
{
    /* --- Calculation of the number of clusters according to the target size --- */

    // NOTE: The Z dimension defined here is the minimum number of slices allocated initially.
    //       During rendering, the actual Z slices are dynamic and calculated per-frame based on
    //       the camera's near and far planes using a logarithmic distribution.

    NX_IVec2 resolution = (desc.render3D.resolution > NX_IVEC2_ONE)
        ? desc.render3D.resolution : NX_GetDisplaySize();

    mClusterSize.x = std::max(16, resolution.x / 80); // 80 px per target cluster
    mClusterSize.y = std::max(9, resolution.y / 50);  // 50 px per target cluster

    mClusterCount.x = NX_DIV_CEIL(resolution.x, mClusterSize.x);
    mClusterCount.y = NX_DIV_CEIL(resolution.y, mClusterSize.y);
    mClusterCount.z = 16;

    int clusterTotal = mClusterCount.x * mClusterCount.y * mClusterCount.z;

    /* --- Create light and shadow storages --- */

    mStorageLights = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        32 * sizeof(NX_Light::LightGPU),
        nullptr, GL_DYNAMIC_DRAW
    );

    mStorageShadow = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        32 * sizeof(NX_Light::ShadowGPU),
        nullptr, GL_DYNAMIC_DRAW
    );

    /* --- Create lihgting cluster storages --- */

    mStorageClusters = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        clusterTotal * 4 * sizeof(uint32_t),
        nullptr, GL_DYNAMIC_COPY
    );

    mStorageIndex = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        clusterTotal * MaxLightsPerCluster * sizeof(uint32_t),
        nullptr, GL_DYNAMIC_COPY
    );

    mStorageClusterAABB = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        clusterTotal * sizeof(NX_BoundingBox),
        nullptr, GL_DYNAMIC_COPY
    );

    /* --- Create shadow maps --- */

    mTargetShadow[NX_LIGHT_DIR] = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D_ARRAY,
            .internalFormat = GL_R16F,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 1,
            .mipmap = false
        }
    );

    mTargetShadow[NX_LIGHT_SPOT] = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D_ARRAY,
            .internalFormat = GL_R16F,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 1,
            .mipmap = false
        }
    );

    mTargetShadow[NX_LIGHT_OMNI] = gpu::Texture(
        gpu::TextureConfig {
            .target = GL_TEXTURE_CUBE_MAP_ARRAY,
            .internalFormat = GL_R16F,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 1,
            .mipmap = false
        }
    );

    mShadowDepth = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_DEPTH_COMPONENT24,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .mipmap = false,
        }
    );

    /* --- Create shadow framebuffers --- */

    for (int i = 0; i < mFramebufferShadow.size(); i++) {
        mFramebufferShadow[i] = gpu::Framebuffer(
            {&mTargetShadow[i]}, &mShadowDepth
        );
    }

    /* --- Reserve caches space --- */

    if (!mActiveLights.reserve(32)) {
        NX_LOG(E, "RENDER: Active lights cache pre-allocation failed (requested: 32 entries)");
    }

    if (!mActiveShadows.reserve(8)) {
        NX_LOG(E, "RENDER: Active shadows cache pre-allocation failed (requested: 8 entries)");
    }

    if (!mShadowNeedingUpdate.reserve(8)) {
        NX_LOG(E, "RENDER: Shadows needing update cache pre-allocation failed (requested: 8 entries)");
    }
}

/* === Private Implementation === */

void LightManager::updateState(const ProcessParams& params)
{
    constexpr size_t NumLightTypes = NX_LIGHT_TYPE_COUNT;

    /* --- Count each active light type --- */

    std::array<size_t, NumLightTypes> counts{};
    for (const NX_Light& light : mLights) {
        if (light.isActive()) {
            ++counts[light.type()];
        }
    }

    mActiveLights.resize(
        counts[NX_LIGHT_DIR] +
        counts[NX_LIGHT_SPOT] +
        counts[NX_LIGHT_OMNI]
    );

    /* --- Prepare offsets for each type --- */

    std::array<size_t, NumLightTypes> offsets{};

    offsets[NX_LIGHT_DIR]  = 0;
    offsets[NX_LIGHT_SPOT] = counts[NX_LIGHT_DIR];
    offsets[NX_LIGHT_OMNI] = counts[NX_LIGHT_DIR] + counts[NX_LIGHT_SPOT];

    /* --- Update and insert active lights --- */

    for (NX_Light& light : mLights)
    {
        if (!light.isActive()) continue;

        bool needsShadowUpdate = false;
        light.updateState(params.viewFrustum, &needsShadowUpdate);

        int32_t shadowIndex = -1;
        if (light.isShadowActive()) {
            shadowIndex = mActiveShadows.size();
            uint32_t mapIndex = mActiveShadows.size(light.type());
            mActiveShadows.emplace(light.type(), &light, mapIndex);
            if (needsShadowUpdate) {
                mShadowNeedingUpdate.emplace(light.type(), shadowIndex);
            }
        }

        size_t& offset = offsets[light.type()];
        mActiveLights[offset++] = ActiveLight(&light, shadowIndex);
    }
}

void LightManager::uploadLights(const ProcessParams& params)
{
    if (mActiveLights.empty()) {
        return;
    }

    mStorageLights.reserve(mLights.size() * sizeof(NX_Light::LightGPU), false);
    NX_Light::LightGPU* mappedLights = mStorageLights.mapRange<NX_Light::LightGPU>(
        0, mActiveLights.size() * sizeof(NX_Light::LightGPU),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    for (int i = 0; i < mActiveLights.size(); i++) {
        const ActiveLight& data = mActiveLights[i];
        data.light->fillLightGPU(&mappedLights[i], data.shadowIndex);
    }

    mStorageLights.unmap();
}

void LightManager::uploadShadows(const ProcessParams& params)
{
    if (mActiveShadows.empty()) {
        return;
    }

    mStorageShadow.reserve(mActiveShadows.size() * sizeof(NX_Light::ShadowGPU), false);
    NX_Light::ShadowGPU* mappedShadows = mStorageShadow.mapRange<NX_Light::ShadowGPU>(
        0, mActiveShadows.size() * sizeof(NX_Light::ShadowGPU),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT
    );

    for (int i = 0; i < mActiveShadows.size(); i++) {
        const ActiveShadow& data = mActiveShadows[i];
        data.light->fillShadowGPU(&mappedShadows[i], data.mapIndex);
    }

    mStorageShadow.unmap();
}

void LightManager::computeClusters(const ProcessParams& params)
{
    /* --- Early exit if no active light --- */

    if (mActiveLights.empty()) {
        return;
    }

    /* --- Adapt the number of clusters in Z according to the view frustum --- */

    // SlicesPerDepthOctave defines how many logarithmically-distributed depth slices are
    // allocated per doubling of distance from the near plane. Higher values increase
    // cluster resolution near the camera, improving light culling precision.

    mClusterCount.z = std::clamp(int(log2(params.viewFrustum.far() / params.viewFrustum.near()) * SlicesPerDepthOctave), 16, 64);
    int clusterTotal = mClusterCount.x * mClusterCount.y * mClusterCount.z;

    mStorageClusters.reserve(clusterTotal * 4 * sizeof(uint32_t), false);
    mStorageIndex.reserve(clusterTotal * MaxLightsPerCluster * sizeof(uint32_t), false);
    mStorageClusterAABB.reserve(clusterTotal * (sizeof(NX_Vec4) + sizeof(NX_Vec3)), false); //< minBounds and maxBounds with padding

    /* --- Calculate the Z-slicing parameters --- */

    mClusterSliceScale = float(mClusterCount.z) / log2(params.viewFrustum.far() / params.viewFrustum.near());
    mClusterSliceBias = -float(mClusterCount.z) * log2(params.viewFrustum.near()) / log2(params.viewFrustum.far() / params.viewFrustum.near());

    /* --- Obtaining the lights affecting each tile --- */

    gpu::Pipeline pipeline;
    pipeline.useProgram(mPrograms.lightCulling());

    pipeline.bindUniform(0, params.viewFrustum.buffer());
    pipeline.bindStorage(0, mStorageLights);
    pipeline.bindStorage(1, mStorageClusters);
    pipeline.bindStorage(2, mStorageIndex);
    pipeline.bindStorage(3, mStorageClusterAABB);

    pipeline.setUniformUint3(0, mClusterCount);
    pipeline.setUniformFloat1(1, mClusterSliceScale);
    pipeline.setUniformFloat1(2, mClusterSliceBias);
    pipeline.setUniformUint1(3, mActiveLights.size());
    pipeline.setUniformUint1(4, MaxLightsPerCluster);

    pipeline.dispatchCompute(
        NX_DIV_CEIL(mClusterCount.x, 4),
        NX_DIV_CEIL(mClusterCount.y, 4),
        NX_DIV_CEIL(mClusterCount.z, 4)
    );
}

void LightManager::renderShadowMaps(const ProcessParams& params)
{
    /* --- Early exit if no shadows to render --- */

    if (mShadowNeedingUpdate.empty()) {
        return;
    }

    /* --- Ensures that there are enough shadow maps in each texture array --- */

    for (int i = 0; i < mTargetShadow.size(); i++) {
        size_t activeCount = mActiveShadows.size(static_cast<NX_LightType>(i));
        if (activeCount > mTargetShadow[i].depth()) {
            mTargetShadow[i].realloc(mTargetShadow[i].width(), mTargetShadow[i].height(), activeCount);
            mFramebufferShadow[i].updateColorTextureView(0, mTargetShadow[i]);
        }
    }

    /* --- Setup pipeline --- */

    gpu::Pipeline pipeline;

    pipeline.setViewport(0, 0, mShadowResolution, mShadowResolution);
    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);

    pipeline.bindStorage(0, params.drawCalls.sharedBuffer());
    pipeline.bindStorage(1, params.drawCalls.uniqueBuffer());
    pipeline.bindStorage(2, params.drawCalls.boneBuffer());

    pipeline.bindUniform(1, params.viewFrustum.buffer());
    pipeline.bindUniform(2, params.environment.buffer());

    /* --- Render shadows for each lights --- */

    for (int i = 0; i < mFramebufferShadow.size(); ++i)
    {
        NX_LightType lightType = static_cast<NX_LightType>(i);
        if (mShadowNeedingUpdate.empty(lightType)) continue;
        pipeline.bindFramebuffer(mFramebufferShadow[lightType]);

        for (uint32_t shadowIndex : mShadowNeedingUpdate.category(lightType))
        {
            const ActiveShadow& data = mActiveShadows[shadowIndex];
            const NX_Light& light = *data.light;

            for (int face = 0; face < ((lightType == NX_LIGHT_OMNI) ? 6 : 1); ++face)
            {
                mFramebufferShadow[lightType].setColorAttachmentTarget(0, data.mapIndex, face);
                pipeline.clear(mFramebufferShadow[lightType], NX_COLOR_1(light.range()));

                // REVIEW: We could use a better method for per-frame data...
                mFrameShadowUniform->uploadObject(FrameShadowUniform {
                    .lightViewProj = light.viewProj(face),
                    .lightPosition = light.position(),
                    .lightRange = light.range(),
                    .lightType = light.type(),
                    .elapsedTime = static_cast<float>(NX_GetElapsedTime())
                });
                pipeline.bindUniform(0, *mFrameShadowUniform);
                mFrameShadowUniform.rotate();

                params.drawCalls.culling(light.frustum(face), light.shadowCullMask());

                for (int uniqueIndex : params.drawCalls.uniqueVisible().categories(DRAW_OPAQUE, DRAW_PREPASS, DRAW_TRANSPARENT))
                {
                    const DrawUnique& unique = params.drawCalls.uniqueData()[uniqueIndex];
                    if (unique.mesh.shadowCastMode() == NX_SHADOW_CAST_DISABLED) continue;

                    NX_MaterialShader& shader = mPrograms.materialShader(unique.material.shader);
                    pipeline.useProgram(shader.program(NX_MaterialShader::Variant::SCENE_SHADOW));
                    pipeline.setCullMode(render::getCullMode(unique.mesh.shadowFaceMode(), unique.material.cull));

                    shader.bindTextures(pipeline, unique.textures, mAssets.textureWhite().gpuTexture());
                    shader.bindUniforms(pipeline, unique.dynamicRangeIndex);

                    pipeline.bindTexture(0, mAssets.textureOrWhite(unique.material.albedo.texture));
                    pipeline.setUniformUint1(0, unique.sharedDataIndex);
                    pipeline.setUniformUint1(1, unique.uniqueDataIndex);

                    params.drawCalls.draw(pipeline, unique);
                }
            }
        }
    }
}

} // namespace scene
