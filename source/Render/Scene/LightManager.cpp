/* LightManager.cpp -- Direct light management class for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./LightManager.hpp"

#include "../../Detail/GPU/Pipeline.hpp"
#include "../HP_Texture.hpp"
#include "./DrawCall.hpp"

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Macros.h>
#include <Hyperion/HP_Math.h>
#include <cfloat>

namespace scene {

/* === Public Implementation === */

LightManager::LightManager(render::ProgramCache& programs, render::AssetCache& assets, HP_IVec2 resolution, int shadowRes)
    : mPrograms(programs), mAssets(assets), mShadowResolution{shadowRes > 0 ? shadowRes : 2048}
{
    /* --- Calculation of the number of clusters according to the target size --- */

    // NOTE: The Z dimension defined here is the minimum number of slices allocated initially.
    //       During rendering, the actual Z slices are dynamic and calculated per-frame based on
    //       the camera's near and far planes using a logarithmic distribution.

    mClusterSize.x = std::max(16, resolution.x / 80); // 80 px per target cluster
    mClusterSize.y = std::max(9, resolution.y / 50);  // 50 px per target cluster

    mClusterCount.x = HP_DIV_CEIL(resolution.x, mClusterSize.x);
    mClusterCount.y = HP_DIV_CEIL(resolution.y, mClusterSize.y);
    mClusterCount.z = 16;

    int clusterTotal = mClusterCount.x * mClusterCount.y * mClusterCount.z;

    /* --- Create light and shadow storages --- */

    mStorageLights = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        32 * sizeof(HP_Light::LightGPU),
        nullptr, GL_DYNAMIC_DRAW
    );

    mStorageShadow = gpu::Buffer(
        GL_SHADER_STORAGE_BUFFER,
        32 * sizeof(HP_Light::ShadowGPU),
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
        clusterTotal * sizeof(HP_BoundingBox),
        nullptr, GL_DYNAMIC_COPY
    );

    /* --- Create shadow maps --- */

    // NOTE:
    // We select the shadow texture format depending on the target platform.
    //
    // - On desktop (OpenGL 4.5 core):
    //   Use RG32F and store the actual exponentials exp(+/- lambda * d).
    //   This gives maximum precision, correct hardware filtering (bilinear/trilinear),
    //   and avoids artifacts. Memory usage is higher, but desktop GPUs handle it.
    //
    // - On GLES / mobile:
    //   RG32F render targets are not guaranteed to be supported, and RG16F is much cheaper.
    //   However, RG16F cannot safely store very large exponentials (values overflow at ~65504).
    //   To avoid this, we store the logarithm form (+/- lambda * d) instead.
    //   At sampling time we reconstruct the exponentials with exp() in the shader.
    //   This adds a small ALU cost and filtering occurs in log-space (slightly different),
    //   but it prevents overflow and keeps memory usage low.

    GLenum shadowFormat = GL_RG32F;
    if (gCore->glProfile() == SDL_GL_CONTEXT_PROFILE_ES) {
        shadowFormat = GL_RG16F;
    }

    // NOTE: Trilinear filtering can be enabled for shadow maps when using EVSM,
    //       as long as mipmaps are generated after rendering the shadows.
    //       However, for Hyperion's current use cases, EVSM with simple
    //       bilinear filtering is already perfectly acceptable.

    mShadowMapCubeArray = gpu::Texture(
        gpu::TextureConfig {
            .target = GL_TEXTURE_CUBE_MAP_ARRAY,
            .internalFormat = shadowFormat,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 2,
            .mipmap = false,
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR
        }
    );

    mShadowMap2DArray = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D_ARRAY,
            .internalFormat = shadowFormat,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 4,
            .mipmap = false,
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
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

    mFramebufferShadowCube = gpu::Framebuffer({&mShadowMapCubeArray}, &mShadowDepth);
    mFramebufferShadow2D = gpu::Framebuffer({&mShadowMap2DArray}, &mShadowDepth);
}

/* === Private Implementation === */

void LightManager::updateState(const ProcessParams& params)
{
    mActiveLightCount =
    mActiveShadow2DCount =
    mActiveShadowCubeCount = 0;

    for (HP_Light& light : mLights)
    {
        if (!light.isActive()) continue;

        uint32_t lightIndex = mActiveLightCount++;
        uint32_t shadowMapIndex = 0;
        int32_t shadowIndex = -1;

        if (light.isShadowActive())
        {
            shadowIndex = mActiveShadow2DCount + mActiveShadowCubeCount;

            switch (light.type()) {
            case HP_LIGHT_DIR:
            case HP_LIGHT_SPOT:
                shadowMapIndex = mActiveShadow2DCount++;
                break;
            case HP_LIGHT_OMNI:
                shadowMapIndex = mActiveShadowCubeCount++;
                break;
            }
        }

        light.updateState(params.environement.bounds, lightIndex, shadowIndex, shadowMapIndex);
    }
}

void LightManager::uploadActive(const ProcessParams& params)
{
    if (!(mLightDirty || mShadowDirty)) {
        return;
    }

    /* --- Calculate shadow counts --- */

    const int shadowCount = mActiveShadow2DCount + mActiveShadowCubeCount;

    /* --- Prepare buffers if necessary --- */

    HP_Light::LightGPU* mappedLights = nullptr;
    HP_Light::ShadowGPU* mappedShadows = nullptr;

    if (mLightDirty && mActiveLightCount > 0) {
        mStorageLights.reserve(mLights.size() * sizeof(HP_Light::LightGPU), false);
        mappedLights = mStorageLights.mapRange<HP_Light::LightGPU>(0, mActiveLightCount * sizeof(HP_Light::LightGPU));
    }

    if (mShadowDirty && shadowCount > 0) {
        mShadowMapCubeArray.reserve(mShadowMapCubeArray.width(), mShadowMapCubeArray.height(), mActiveShadowCubeCount);
        mShadowMap2DArray.reserve(mShadowMap2DArray.width(), mShadowMap2DArray.height(), mActiveShadow2DCount);
        mStorageShadow.reserve(shadowCount * sizeof(HP_Light::ShadowGPU), false);
        mappedShadows = mStorageShadow.mapRange<HP_Light::ShadowGPU>(0, shadowCount * sizeof(HP_Light::ShadowGPU));
    }

    /* --- Light and shadow processing loop --- */

    for (HP_Light& light : mLights) {
        if (!light.isActive()) {
            continue;
        }
        if (mappedLights) {
            light.fillLightGPU(&mappedLights[light.lightIndex()]);
        }
        if (mappedShadows && light.isShadowActive()) {
            light.fillShadowGPU(&mappedShadows[light.shadowIndex()]);
        }
    }

    /* --- Unmap buffers --- */

    if (mappedLights) {
        mStorageLights.unmap();
        HP_INTERNAL_LOG(V, "RENDER: %i lights have been uploaded", mActiveLightCount);
        mLightDirty = false;
    }

    if (mappedShadows) {
        mStorageShadow.unmap();
        HP_INTERNAL_LOG(V, "RENDER: %i shadows have been uploaded", shadowCount);
        mShadowDirty = false;
    }
}

void LightManager::computeClusters(const ProcessParams& params)
{
    /* --- Early exit if no active light --- */

    if (mActiveLightCount == 0) {
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
    mStorageClusterAABB.reserve(clusterTotal * (sizeof(HP_Vec4) + sizeof(HP_Vec3)), false); //< minBounds and maxBounds with padding

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
    pipeline.setUniformUint1(3, mActiveLightCount);
    pipeline.setUniformUint1(4, MaxLightsPerCluster);

    pipeline.dispatchCompute(
        HP_DIV_CEIL(mClusterCount.x, 4),
        HP_DIV_CEIL(mClusterCount.y, 4),
        HP_DIV_CEIL(mClusterCount.z, 4)
    );
}

void LightManager::renderShadowMaps(const ProcessParams& params)
{
    /* --- Early exit if no active light --- */

    int shadowCount = (mActiveShadowCubeCount + mActiveShadow2DCount);
    if (shadowCount == 0) {
        return;
    }

    /* --- Lambda for drawing calls --- */

    const auto draw = [this, &params](const gpu::Pipeline& pipeline, const DrawCall& call, const DrawData& data)
    {
        const HP_Texture* texture = call.material().albedo.texture;
        pipeline.bindTexture(0, mAssets.textureOrWhite(texture));

        pipeline.setUniformMat4(1, data.matrix());
        pipeline.setUniformFloat2(2, call.material().texOffset);
        pipeline.setUniformFloat2(3, call.material().texScale);
        pipeline.setUniformFloat1(4, call.material().albedo.color.a);
        pipeline.setUniformInt1(5, data.isAnimated());
        pipeline.setUniformInt1(6, data.boneMatrixOffset());
        pipeline.setUniformInt1(7, data.useInstancing());
        pipeline.setUniformUint1(8, call.material().billboard);
        pipeline.setUniformFloat1(11, call.material().alphaCutOff);

        switch (call.mesh().shadowFaceMode) {
        case HP_SHADOW_FACE_AUTO:
            switch (call.material().cull) {
            case HP_CULL_BACK:
                pipeline.setCullMode(gpu::CullMode::Back);
                break;
            case HP_CULL_FRONT:
                pipeline.setCullMode(gpu::CullMode::Front);
                break;
            case HP_CULL_NONE:
                pipeline.setCullMode(gpu::CullMode::Disabled);
                break;
            }
            break;
        case HP_SHADOW_FACE_FRONT:
            pipeline.setCullMode(gpu::CullMode::Back);
            break;
        case HP_SHADOW_FACE_BACK:
            pipeline.setCullMode(gpu::CullMode::Front);
            break;
        case HP_SHADOW_FACE_BOTH:
            pipeline.setCullMode(gpu::CullMode::Disabled);
            break;
        }

        call.draw(pipeline, data.instances(), data.instanceCount());
    };

    /* --- Pipeline initialization --- */

    gpu::Pipeline pipeline;

    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.setCullMode(gpu::CullMode::Back);

    pipeline.setViewport(0, 0, mShadowResolution, mShadowResolution);
    pipeline.useProgram(mPrograms.shadow());

    /* --- Bind UBOs and SSBOs --- */

    pipeline.bindUniform(0, params.viewFrustum.buffer());
    pipeline.bindStorage(0, params.boneBuffer.buffer());

    /* --- Iterates through all lights with shadows active --- */

    for (HP_Light& light : mLights)
    {
        if (!light.isActive() || !light.isShadowActive() || !light.needsShadowMapUpdate()) {
            continue;
        }

        pipeline.setUniformFloat3(10, light.position());
        pipeline.setUniformFloat1(12, light.shadowLambda());
        pipeline.setUniformFloat1(13, light.range());

        float rangeSq = HP_POW2(light.range());

        switch (light.type()) {
        case HP_LIGHT_DIR:
            pipeline.bindFramebuffer(mFramebufferShadow2D);
            mFramebufferShadow2D.setColorAttachmentTarget(0, light.shadowIndex());
            pipeline.clear(mFramebufferShadow2D, HP_COLOR_1(FLT_MAX));
            pipeline.setUniformMat4(0, light.viewProj());
            for (const DrawCall& call : params.drawCalls.categories(DrawCall::OPAQUE, DrawCall::TRANSPARENT))
            {
                if (call.mesh().shadowCastMode == HP_SHADOW_CAST_DISABLED) continue;
                if ((light.shadowCullMask() & call.mesh().layerMask) == 0) continue;

                if (light.isInsideShadowFrustum(call, params.drawData[call.dataIndex()])) {
                    draw(pipeline, call, params.drawData[call.dataIndex()]);
                }
            }
            break;
        case HP_LIGHT_SPOT:
            pipeline.bindFramebuffer(mFramebufferShadow2D);
            mFramebufferShadow2D.setColorAttachmentTarget(0, light.shadowIndex());
            pipeline.clear(mFramebufferShadow2D, HP_COLOR_1(FLT_MAX));
            pipeline.setUniformMat4(0, light.viewProj());
            for (const DrawCall& call : params.drawCalls.categories(DrawCall::OPAQUE, DrawCall::TRANSPARENT))
            {
                if (call.mesh().shadowCastMode == HP_SHADOW_CAST_DISABLED) continue;
                if ((light.shadowCullMask() & call.mesh().layerMask) == 0) continue;

                if (light.isInsideShadowFrustum(call, params.drawData[call.dataIndex()])) {
                    draw(pipeline, call, params.drawData[call.dataIndex()]);
                }
            }
            break;
        case HP_LIGHT_OMNI:
            pipeline.bindFramebuffer(mFramebufferShadowCube);
            for (int iFace = 0; iFace < 6; iFace++) {
                mFramebufferShadowCube.setColorAttachmentTarget(0, light.shadowIndex(), iFace);
                pipeline.clear(mFramebufferShadowCube, HP_COLOR_1(FLT_MAX));
                pipeline.setUniformMat4(0, light.viewProj(iFace));
                for (const DrawCall& call : params.drawCalls.categories(DrawCall::OPAQUE, DrawCall::TRANSPARENT))
                {
                    if (call.mesh().shadowCastMode == HP_SHADOW_CAST_DISABLED) continue;
                    if ((light.shadowCullMask() & call.mesh().layerMask) == 0) continue;

                    if (light.isInsideShadowFrustum(call, params.drawData[call.dataIndex()], iFace)) {
                        draw(pipeline, call, params.drawData[call.dataIndex()]);
                    }
                }
            }
            break;
        }
    }
}

} // namespace scene
