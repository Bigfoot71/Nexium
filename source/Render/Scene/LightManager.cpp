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

LightManager::LightManager(render::ProgramCache& programs, render::AssetCache& assets, const HP_AppDesc& desc)
    : mPrograms(programs), mAssets(assets)
    , mFrameShadowUniform(GL_UNIFORM_BUFFER, sizeof(FrameShadowUniform), nullptr, GL_DYNAMIC_DRAW)
    , mShadowResolution{desc.render3D.shadowRes > 0 ? desc.render3D.shadowRes : 2048}
{
    /* --- Calculation of the number of clusters according to the target size --- */

    // NOTE: The Z dimension defined here is the minimum number of slices allocated initially.
    //       During rendering, the actual Z slices are dynamic and calculated per-frame based on
    //       the camera's near and far planes using a logarithmic distribution.

    HP_IVec2 resolution = (desc.render3D.resolution > HP_IVEC2_ONE)
        ? desc.render3D.resolution : HP_GetDisplaySize();

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

    mShadowMapCubeArray = gpu::Texture(
        gpu::TextureConfig {
            .target = GL_TEXTURE_CUBE_MAP_ARRAY,
            .internalFormat = shadowFormat,
            .width = mShadowResolution,
            .height = mShadowResolution,
            .depth = 1,
            .mipmap = desc.render3D.shadowCubeMip,
        },
        gpu::TextureParam
        {
            .minFilter = static_cast<GLenum>(desc.render3D.shadowCubeMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR),
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
            .depth = 1,
            .mipmap = desc.render3D.shadow2DMip,
        },
        gpu::TextureParam
        {
            .minFilter = static_cast<GLenum>(desc.render3D.shadow2DMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR),
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

        light.updateState(params.environment.bounds(), lightIndex, shadowIndex, shadowMapIndex);
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

    if (mShadowDirty && shadowCount > 0)
    {
        mStorageShadow.reserve(shadowCount * sizeof(HP_Light::ShadowGPU), false);
        mappedShadows = mStorageShadow.mapRange<HP_Light::ShadowGPU>(0, shadowCount * sizeof(HP_Light::ShadowGPU));

        if (mActiveShadowCubeCount > mShadowMapCubeArray.depth()) {
            mShadowMapCubeArray.realloc(mShadowMapCubeArray.width(), mShadowMapCubeArray.height(), mActiveShadowCubeCount);
            mFramebufferShadowCube.updateColorTextureView(0, mShadowMapCubeArray);
        }

        if (mActiveShadow2DCount > mShadowMap2DArray.depth()) {
            mShadowMap2DArray.reserve(mShadowMap2DArray.width(), mShadowMap2DArray.height(), mActiveShadow2DCount);
            mFramebufferShadow2D.updateColorTextureView(0, mShadowMap2DArray);
        }
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
        HP_MaterialShader& shader = mPrograms.materialShader(call.material().shader);

        pipeline.useProgram(shader.program(HP_MaterialShader::SCENE_SHADOW));
        shader.bindUniformBuffers(pipeline, HP_MaterialShader::SCENE_SHADOW, call.dynamicRangeIndex());
        shader.bindTextures(pipeline, call.materialShaderTextures(), mAssets.textureWhite().gpuTexture());

        const HP_Texture* texture = call.material().albedo.texture;
        pipeline.bindTexture(0, mAssets.textureOrWhite(texture));

        params.renderableBuffer.upload(data, call);
        params.materialBuffer.upload(call.material());

        pipeline.bindUniform(3, params.renderableBuffer.buffer());
        pipeline.bindUniform(4, params.materialBuffer.buffer());

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

    pipeline.setViewport(0, 0, mShadowResolution, mShadowResolution);
    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);

    /* --- Bind UBOs and SSBOs --- */

    pipeline.bindStorage(0, params.boneBuffer.buffer());
    pipeline.bindUniform(1, params.viewFrustum.buffer());
    pipeline.bindUniform(2, params.environment.buffer());

    /* --- Iterates through all lights with shadows active --- */

    const bool frustumCulling = params.environment.hasFlags(HP_ENV_SHADOW_FRUSTUM_CULLING);

    int updatedCubeCount = 0;
    int updated2DCount = 0;

    for (HP_Light& light : mLights)
    {
        if (!light.isActive() || !light.isShadowActive() || !light.needsShadowMapUpdate()) {
            continue;
        }

        float rangeSq = HP_POW2(light.range());

        switch (light.type()) {
        case HP_LIGHT_DIR:
            {
                updated2DCount++;

                mFrameShadowUniform->uploadObject(FrameShadowUniform {
                    .lightViewProj = light.viewProj(),
                    .lightPosition = light.position(),
                    .shadowLambda = light.shadowLambda(),
                    .farPlane = light.range(),
                    .elapsedTime = static_cast<float>(HP_GetElapsedTime())
                });
                pipeline.bindUniform(0, *mFrameShadowUniform);

                pipeline.bindFramebuffer(mFramebufferShadow2D);
                mFramebufferShadow2D.setColorAttachmentTarget(0, light.shadowIndex());
                pipeline.clear(mFramebufferShadow2D, HP_COLOR_1(FLT_MAX));

                for (const DrawCall& call : params.drawCalls.categories(
                    DrawCall::OPAQUE, DrawCall::PREPASS, DrawCall::TRANSPARENT))
                {
                    if (call.mesh().shadowCastMode == HP_SHADOW_CAST_DISABLED) continue;
                    if ((light.shadowCullMask() & call.mesh().layerMask) == 0) continue;

                    if (!frustumCulling || light.isInsideShadowFrustum(call, params.drawData[call.drawDataIndex()])) {
                        draw(pipeline, call, params.drawData[call.drawDataIndex()]);
                    }
                }

                mFrameShadowUniform.rotate();
            }
            break;
        case HP_LIGHT_SPOT:
            {
                updated2DCount++;

                mFrameShadowUniform->uploadObject(FrameShadowUniform {
                    .lightViewProj = light.viewProj(),
                    .lightPosition = light.position(),
                    .shadowLambda = light.shadowLambda(),
                    .farPlane = light.range(),
                    .elapsedTime = static_cast<float>(HP_GetElapsedTime())
                });
                pipeline.bindUniform(0, *mFrameShadowUniform);

                pipeline.bindFramebuffer(mFramebufferShadow2D);
                mFramebufferShadow2D.setColorAttachmentTarget(0, light.shadowIndex());
                pipeline.clear(mFramebufferShadow2D, HP_COLOR_1(FLT_MAX));

                for (const DrawCall& call : params.drawCalls.categories(
                    DrawCall::OPAQUE, DrawCall::PREPASS, DrawCall::TRANSPARENT))
                {
                    if (call.mesh().shadowCastMode == HP_SHADOW_CAST_DISABLED) continue;
                    if ((light.shadowCullMask() & call.mesh().layerMask) == 0) continue;

                    if (!frustumCulling || light.isInsideShadowFrustum(call, params.drawData[call.drawDataIndex()])) {
                        draw(pipeline, call, params.drawData[call.drawDataIndex()]);
                    }
                }

                mFrameShadowUniform.rotate();
            }
            break;
        case HP_LIGHT_OMNI:
            {
                updatedCubeCount++;
                pipeline.bindFramebuffer(mFramebufferShadowCube);

                for (int iFace = 0; iFace < 6; iFace++)
                {
                    mFrameShadowUniform->uploadObject(FrameShadowUniform {
                        .lightViewProj = light.viewProj(iFace),
                        .lightPosition = light.position(),
                        .shadowLambda = light.shadowLambda(),
                        .farPlane = light.range(),
                        .elapsedTime = static_cast<float>(HP_GetElapsedTime())
                    });
                    pipeline.bindUniform(0, *mFrameShadowUniform);

                    mFramebufferShadowCube.setColorAttachmentTarget(0, light.shadowIndex(), iFace);
                    pipeline.clear(mFramebufferShadowCube, HP_COLOR_1(FLT_MAX));

                    for (const DrawCall& call : params.drawCalls.categories(
                        DrawCall::OPAQUE, DrawCall::PREPASS, DrawCall::TRANSPARENT))
                    {
                        if (call.mesh().shadowCastMode == HP_SHADOW_CAST_DISABLED) continue;
                        if ((light.shadowCullMask() & call.mesh().layerMask) == 0) continue;

                        if (!frustumCulling || light.isInsideShadowFrustum(call, params.drawData[call.drawDataIndex()], iFace)) {
                            draw(pipeline, call, params.drawData[call.drawDataIndex()]);
                        }
                    }

                    mFrameShadowUniform.rotate();
                }
            }
            break;
        }
    }

    /* --- Update mipmaps if necessary --- */

    if (mShadowMapCubeArray.numLevels() > 1 && updatedCubeCount > 0) {
        mShadowMapCubeArray.generateMipmap();
    }

    if (mShadowMap2DArray.numLevels() > 1 && updated2DCount > 0) {
        mShadowMap2DArray.generateMipmap();
    }
}

} // namespace scene
