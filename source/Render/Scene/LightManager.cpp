/* LightManager.cpp -- Direct light management class for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./LightManager.hpp"

#include "../../Detail/GPU/Pipeline.hpp"
#include "../NX_Texture.hpp"
#include "./DrawCall.hpp"

#include <NX/NX_Render.h>
#include <NX/NX_Macros.h>
#include <NX/NX_Math.h>
#include <cfloat>

namespace scene {

/* === Public Implementation === */

LightManager::LightManager(render::ProgramCache& programs, render::AssetCache& assets, const NX_AppDesc& desc)
    : mPrograms(programs), mAssets(assets)
    , mFrameShadowUniform(GL_UNIFORM_BUFFER, sizeof(FrameShadowUniform), nullptr, GL_DYNAMIC_DRAW)
    , mShadowResolution{desc.render3D.shadowRes > 0 ? desc.render3D.shadowRes : 1024}
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
        clusterTotal * sizeof(uint32_t),
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

    GLenum shadowFormat = GL_RG32F;
    if (gCore->glProfile() == SDL_GL_CONTEXT_PROFILE_ES) {
        shadowFormat = GL_RG16F;
    }

    mTargetShadow[NX_LIGHT_DIR] = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D_ARRAY,
            .internalFormat = shadowFormat,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 1,
            .mipmap = desc.render3D.shadowDirMip,
        },
        gpu::TextureParam
        {
            .minFilter = static_cast<GLenum>(desc.render3D.shadowDirMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR),
            .magFilter = GL_LINEAR
        }
    );

    mTargetShadow[NX_LIGHT_SPOT] = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D_ARRAY,
            .internalFormat = shadowFormat,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 1,
            .mipmap = desc.render3D.shadowSpotMip,
        },
        gpu::TextureParam
        {
            .minFilter = static_cast<GLenum>(desc.render3D.shadowSpotMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR),
            .magFilter = GL_LINEAR
        }
    );

    mTargetShadow[NX_LIGHT_OMNI] = gpu::Texture(
        gpu::TextureConfig {
            .target = GL_TEXTURE_CUBE_MAP_ARRAY,
            .internalFormat = shadowFormat,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 1,
            .mipmap = desc.render3D.shadowOmniMip,
        },
        gpu::TextureParam
        {
            .minFilter = static_cast<GLenum>(desc.render3D.shadowOmniMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR),
            .magFilter = GL_LINEAR
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

    /* --- Create pre-blur framebuffer --- */

    mTargetPreBlur = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = shadowFormat,
            .width = mShadowResolution,
            .height = mShadowResolution
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    mFramebufferPreBlur = gpu::Framebuffer(
        {&mTargetPreBlur}
    );
}

/* === Private Implementation === */

void LightManager::updateState(const ProcessParams& params)
{
    for (NX_Light& light : mLights)
    {
        if (!light.isActive()) {
            continue;
        }

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

        mActiveLights.emplace(light.type(), &light, shadowIndex);
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

    mStorageClusters.reserve(clusterTotal * sizeof(uint32_t), false);
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

    /* --- Lambda to get the clear value --- */

    const auto clearValue = [&](const NX_Light& light) {
        if (gCore->glProfile() == SDL_GL_CONTEXT_PROFILE_ES) return NX_COLOR(1.0f, 1.0f, 0.0f, 0.0f);
        else return NX_COLOR(expf(+light.shadowLambda()), expf(-light.shadowLambda()), 0.0f, 0.0f);
    };

    /* --- Lambda for drawing shadow geometry --- */

    const auto draw = [this, &params](const gpu::Pipeline& pipeline, const DrawCall& call, const DrawData& data)
    {
        NX_MaterialShader& shader = mPrograms.materialShader(call.material().shader);

        pipeline.useProgram(shader.program(NX_MaterialShader::Variant::SCENE_SHADOW));
        pipeline.setCullMode(render::getCullMode(call.shadowFaceMode(), call.material().cull));

        shader.bindTextures(pipeline, call.materialShaderTextures(), mAssets.textureWhite().gpuTexture());
        shader.bindUniforms(pipeline, call.dynamicRangeIndex());

        pipeline.bindTexture(0, mAssets.textureOrWhite(call.material().albedo.texture));

        pipeline.setUniformUint1(0, data.modelDataIndex());
        pipeline.setUniformUint1(1, call.meshDataIndex());

        call.draw(pipeline, data.instances(), data.instanceCount());
    };

    /* --- Lambda for shadow uniform setup --- */

    const auto setupShadowUniform = [this, &params](gpu::Pipeline& pipeline, NX_Light& light, int faceIndex)
    {
        mFrameShadowUniform->uploadObject(FrameShadowUniform {
            .lightViewProj = light.viewProj(faceIndex),
            .lightPosition = light.position(),
            .shadowLambda = light.shadowLambda(),
            .lightRange = light.range(),
            .elapsedTime = static_cast<float>(NX_GetElapsedTime())
        });
        pipeline.bindUniform(0, *mFrameShadowUniform);
    };

    /* --- Lambda for rendering draw calls for a light --- */

    const auto renderDrawCalls = [this, &params, &draw](gpu::Pipeline& pipeline, const NX_Light& light, int faceIndex)
    {
        const bool frustumCulling = params.environment.hasFlags(NX_ENV_SHADOW_FRUSTUM_CULLING);
        for (const DrawCall& call : params.drawCalls.categories(DrawCall::OPAQUE, DrawCall::PREPASS, DrawCall::TRANSPARENT)) {
            if (call.shadowCastMode() == NX_SHADOW_CAST_DISABLED) continue;
            if ((light.shadowCullMask() & call.layerMask()) == 0) continue;
            const DrawData& data = params.drawData[call.drawDataIndex()];
            if (!frustumCulling || light.isInsideShadowFrustum(call, data, faceIndex)) {
                draw(pipeline, call, data);
            }
        }
    };

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

    pipeline.bindStorage(0, params.perModelBuffer.buffer());
    pipeline.bindStorage(1, params.perMeshBuffer.buffer());
    pipeline.bindStorage(2, params.boneBuffer.buffer());

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

            for (int face = 0; face < ((lightType == NX_LIGHT_OMNI) ? 6 : 1); ++face) {
                mFramebufferShadow[lightType].setColorAttachmentTarget(0, data.mapIndex, face);
                pipeline.clear(mFramebufferShadow[lightType], clearValue(*data.light));
                setupShadowUniform(pipeline, *data.light, face);
                renderDrawCalls(pipeline, *data.light, face);
                mFrameShadowUniform.rotate();
            }
        }
    }
}

void LightManager::preBlursShadowMaps(const ProcessParams& params)
{
    // NOTE: For cubemaps, we simply iterate over each face and
    //       apply a fullscreen blur independently on each face
    //       as if it were a regular texture. Technically, this
    //       doesn't handle the cubemap borders, there are tricks
    //       to deal with that, but so far no noticeable artifacts
    //       or major visual issues have appeared for the shadows,
    //       so as long as it works, we'll keep it this way!

    if (mShadowNeedingUpdate.empty()) {
        return;
    }

    gpu::Pipeline pipeline;

    pipeline.setViewport(0, 0, mShadowResolution, mShadowResolution);
    pipeline.setDepthMode(gpu::DepthMode::Disabled);

    for (int i = 0; i < mFramebufferShadow.size(); ++i)
    {
        NX_LightType lightType = static_cast<NX_LightType>(i);
        bool isOmni = (lightType == NX_LIGHT_OMNI);

        for (uint32_t shadowIndex : mShadowNeedingUpdate.category(lightType))
        {
            const ActiveShadow& data = mActiveShadows[shadowIndex];

            if (data.light->shadowSoftness() <= 0.0f) {
                continue;
            }

            for (int face = 0; face < (isOmni ? 6 : 1); ++face)
            {
                /* --- First pass (horizontal) --- */

                pipeline.useProgram(mPrograms.shadowBilateralBlur(true, isOmni));
                pipeline.bindFramebuffer(mFramebufferPreBlur);

                pipeline.bindTexture(0, mTargetShadow[lightType]);

                pipeline.setUniformFloat1(2, data.light->shadowSoftness());
                if (isOmni) pipeline.setUniformInt1(1, face);
                pipeline.setUniformInt1(0, data.mapIndex);

                pipeline.draw(GL_TRIANGLES, 3);

                /* --- Second pass (vertical) --- */

                pipeline.useProgram(mPrograms.shadowBilateralBlur(false, isOmni));

                pipeline.bindFramebuffer(mFramebufferShadow[i]);
                mFramebufferShadow[lightType].setColorAttachmentTarget(0, data.mapIndex, face);

                pipeline.bindTexture(0, mTargetPreBlur);
                pipeline.setUniformFloat1(2, data.light->shadowSoftness());

                pipeline.draw(GL_TRIANGLES, 3);
            }
        }

        /* --- Generates mipmaps if needed --- */

        if (mTargetShadow[lightType].numLevels() > 1) {
            mTargetShadow[lightType].generateMipmap();
        }
    }
}

} // namespace scene
