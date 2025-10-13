/* Scene.cpp -- Scene system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./Scene.hpp"

#include <NX/NX_Render.h>
#include <NX/NX_Core.h>

#include "../../Detail/BuildInfo.hpp"
#include "../NX_ReflectionProbe.hpp"
#include "../NX_Cubemap.hpp"
#include "../NX_Texture.hpp"

namespace scene {

/* === Public Implementation === */

Scene::Scene(render::ProgramCache& programs, render::AssetCache& assets, NX_AppDesc& desc)
    : mPrograms(programs), mAssets(assets), mLights(programs, assets, desc), mFrustum()
    , mFrameUniform(GL_UNIFORM_BUFFER, sizeof(FrameUniform), nullptr, GL_DYNAMIC_DRAW)
{
    /* --- Tweak description --- */

    if (desc.render3D.resolution < NX_IVEC2_ONE) {
        desc.render3D.resolution = NX_GetDisplaySize();
    }

    desc.render3D.sampleCount = NX_MAX(desc.render3D.sampleCount, 1);

    /* --- Create render targets --- */

    mTargetSceneColor = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGBA16F,
            .data = nullptr,
            .width = desc.render3D.resolution.x,
            .height = desc.render3D.resolution.y
        }
    );

    mTargetSceneNormal = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RG8,
            .data = nullptr,
            .width = desc.render3D.resolution.x,
            .height = desc.render3D.resolution.y
        }
    );

    mTargetSceneDepth = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_DEPTH_COMPONENT24,
            .data = nullptr,
            .width = desc.render3D.resolution.x,
            .height = desc.render3D.resolution.y
        }
    );

    /* --- Configure scene framebuffer --- */

    mFramebufferScene = gpu::Framebuffer(
        { &mTargetSceneColor, &mTargetSceneNormal },
        &mTargetSceneDepth
    );

    if (desc.render3D.sampleCount > 1) {
        mFramebufferScene.setSampleCount(desc.render3D.sampleCount);
    }

    /* --- Create mip chain --- */

    mMipChain = gpu::MipBuffer(
        desc.render3D.resolution.x / 2,
        desc.render3D.resolution.y / 2,
        GL_RGB16F
    );

    /* --- Create swap buffers --- */

    mSwapPostProcess = gpu::SwapBuffer(
        desc.render3D.resolution.x,
        desc.render3D.resolution.y,
        GL_RGB16F
    );

    mSwapAuxiliary = gpu::SwapBuffer(
        desc.render3D.resolution.x / 2,
        desc.render3D.resolution.y / 2,
        GL_RGB16F
    );

    /* --- Reserve enough space for the draw calls array --- */

    if (!mDrawCalls.reserve(1024)) {
        NX_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the draw call buffer");
    }

    if (!mDrawData.reserve(1024)) {
        NX_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the draw data buffer");
    }
}

void Scene::begin(const NX_Camera& camera, const NX_Environment& env, const NX_RenderTexture* target)
{
    /* --- Get target (where we blit) info --- */

    mTargetInfo.target = target;
    mTargetInfo.resolution = (target) ? target->framebuffer().dimensions() : NX_GetWindowSize();
    mTargetInfo.aspect = static_cast<float>(mTargetInfo.resolution.x) / mTargetInfo.resolution.y;

    /* --- Update managers --- */

    mFrustum.update(camera, mTargetInfo.aspect);
    mEnvironment.update(env, mMipChain.numLevels());
}

void Scene::end()
{
    /* --- Process lights --- */

    mLights.process({
        .viewFrustum = mFrustum,
        .environment = mEnvironment,
        .renderableBuffer = mRenderableBuffer,
        .materialBuffer = mMaterialBuffer,
        .boneBuffer = mBoneBuffer,
        .drawCalls = mDrawCalls,
        .drawData = mDrawData
    });

    /* --- Upload frame info data --- */

    mFrameUniform.uploadObject(FrameUniform{
        .screenSize = mFramebufferScene.dimensions(),
        .clusterCount = mLights.clusterCount(),
        .maxLightsPerCluster = static_cast<uint32_t>(mLights.maxLightsPerCluster()),
        .clusterSliceScale = mLights.clusterSliceScale(),
        .clusterSliceBias = mLights.clusterSliceBias(),
        .elapsedTime = static_cast<float>(NX_GetElapsedTime()),
        .hasActiveLights = (mLights.activeCount() > 0),
        .hasProbe = (mEnvironment.skyProbe() != nullptr)
    });

    /* --- View layer/furstum culling --- */

    mDrawCalls.remove_if([this](const DrawCall& call) {
        if ((mFrustum.cullMask() & call.layerMask()) == 0) {
            return true;
        }
        const DrawData& data = mDrawData[call.drawDataIndex()];
        if (!data.useInstancing() && mEnvironment.hasFlags(NX_ENV_VIEW_FRUSTUM_CULLING)) {
            return !mFrustum.containsObb(call.aabb(), data.transform());
        }
        return false;
    });

    /* --- Sort draw calls --- */

    if (mEnvironment.hasFlags(NX_ENV_SORT_OPAQUE)) {
        mDrawCalls.sort(DrawCall::Category::OPAQUE,
            [this](const DrawCall& a, const DrawCall& b) {
                float maxDistA = mFrustum.getDistanceSquaredToCenterPoint(a.aabb(), mDrawData[a.drawDataIndex()].matrix());
                float maxDistB = mFrustum.getDistanceSquaredToCenterPoint(b.aabb(), mDrawData[b.drawDataIndex()].matrix());
                return maxDistA < maxDistB;
            }
        );
    }

    if (mEnvironment.hasFlags(NX_ENV_SORT_TRANSPARENT)) {
        mDrawCalls.sort(DrawCall::Category::TRANSPARENT,
            [this](const DrawCall& a, const DrawCall& b) {
                float maxDistA = mFrustum.getDistanceSquaredToFarthestPoint(a.aabb(), mDrawData[a.drawDataIndex()].matrix());
                float maxDistB = mFrustum.getDistanceSquaredToFarthestPoint(b.aabb(), mDrawData[b.drawDataIndex()].matrix());
                return maxDistA > maxDistB;
            }
        );
    }

    /* --- Render scene --- */

    gpu::Pipeline([this](const gpu::Pipeline& pipeline) { // NOLINT(bugprone-unused-raii)
        renderBackground(pipeline);
        renderPrePass(pipeline);
        renderScene(pipeline);
    });

    mFramebufferScene.resolve();

    /* --- Post process --- */

    const gpu::Texture* source = &mTargetSceneColor;

    if (mEnvironment.isSsaoEnabled()) {
        source = &postSSAO(*source);
    }

    if (mEnvironment.bloomMode() != NX_BLOOM_DISABLED) {
        source = &postBloom(*source);
    }

    postFinal(*source);

    /* --- Clear dynamic uniform buffers --- */

    mPrograms.clearDynamicMaterialBuffers();

    /* --- Reset state --- */

    mBoneBuffer.clear();
    mDrawCalls.clear();
    mDrawData.clear();
}

/* === Private Implementation === */

void Scene::renderBackground(const gpu::Pipeline& pipeline)
{
    pipeline.bindFramebuffer(mFramebufferScene);
    pipeline.setDepthMode(gpu::DepthMode::WriteOnly);
    pipeline.setViewport(mFramebufferScene);

    pipeline.clearDepth(1.0f);
    pipeline.clearColor(0, mEnvironment.background());
    pipeline.clearColor(1, NX_COLOR(0.25f, 0.25f, 1.0f, 1.0f));

    if (mEnvironment.skyCubemap() == nullptr) {
        return;
    }

    mFramebufferScene.setDrawBuffers({0});

    pipeline.bindUniform(1, mFrustum.buffer());
    pipeline.bindUniform(2, mEnvironment.buffer());

    pipeline.setDepthMode(gpu::DepthMode::Disabled);
    pipeline.useProgram(mPrograms.skybox());

    pipeline.bindTexture(0, mEnvironment.skyCubemap()->texture());
    pipeline.draw(GL_TRIANGLES, 36);

    mFramebufferScene.enableAllDrawBuffers();
}

void Scene::renderPrePass(const gpu::Pipeline& pipeline)
{
    if (mDrawCalls.category(DrawCall::PREPASS).empty()) {
        return;
    }

    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.setColorWrite(gpu::ColorWrite::Disabled);

    pipeline.bindStorage(0, mBoneBuffer.buffer());

    pipeline.bindUniform(0, mFrameUniform);
    pipeline.bindUniform(1, mFrustum.buffer());
    pipeline.bindUniform(2, mEnvironment.buffer());

    for (const DrawCall& call : mDrawCalls.category(DrawCall::PREPASS))
    {
        const DrawData& data = mDrawData[call.drawDataIndex()];
        const NX_Material& mat = call.material();

        NX_MaterialShader& shader = mPrograms.materialShader(mat.shader);
        pipeline.useProgram(shader.program(NX_MaterialShader::Variant::SCENE_PREPASS));

        shader.bindUniformBuffers(pipeline, call.dynamicRangeIndex());
        shader.bindTextures(pipeline, call.materialShaderTextures(), mAssets.textureWhite().gpuTexture());

        pipeline.setDepthFunc(render::getDepthFunc(mat.depth.test));
        pipeline.setCullMode(render::getCullMode(mat.cull));

        mMaterialBuffer.upload(mat);
        mRenderableBuffer.upload(data, call);

        pipeline.bindTexture(0, mAssets.textureOrWhite(mat.albedo.texture));
        pipeline.bindUniform(3, mRenderableBuffer.buffer());
        pipeline.bindUniform(4, mMaterialBuffer.buffer());

        call.draw(pipeline, data.instances(), data.instanceCount());
    }
}

void Scene::renderScene(const gpu::Pipeline& pipeline)
{
    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.setColorWrite(gpu::ColorWrite::RGBA);

    pipeline.bindStorage(0, mBoneBuffer.buffer());
    pipeline.bindStorage(1, mLights.lightsBuffer());
    pipeline.bindStorage(2, mLights.shadowBuffer());
    pipeline.bindStorage(3, mLights.tilesBuffer());
    pipeline.bindStorage(4, mLights.indexBuffer());

    pipeline.bindTexture(4, mAssets.textureBrdfLut());
    pipeline.bindTexture(7, mLights.shadowCube());
    pipeline.bindTexture(8, mLights.shadow2D());

    pipeline.bindUniform(0, mFrameUniform);
    pipeline.bindUniform(1, mFrustum.buffer());
    pipeline.bindUniform(2, mEnvironment.buffer());

    if (mEnvironment.skyProbe() != nullptr) {
        pipeline.bindTexture(5, mEnvironment.skyProbe()->irradiance());
        pipeline.bindTexture(6, mEnvironment.skyProbe()->prefilter());
    }

    // Ensures SSBOs are ready (especially clusters)
    pipeline.memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    for (const DrawCall& call : mDrawCalls.categories(
        DrawCall::OPAQUE, DrawCall::PREPASS, DrawCall::TRANSPARENT))
    {
        const DrawData& data = mDrawData[call.drawDataIndex()];
        const NX_Material& mat = call.material();

        NX_MaterialShader& shader = mPrograms.materialShader(mat.shader);
        pipeline.useProgram(shader.programFromShadingMode(call.material().shading));

        shader.bindUniformBuffers(pipeline, call.dynamicRangeIndex());
        shader.bindTextures(pipeline, call.materialShaderTextures(), mAssets.textureWhite().gpuTexture());

        pipeline.setDepthFunc(mat.depth.prePass ? gpu::DepthFunc::Equal : render::getDepthFunc(mat.depth.test));
        pipeline.setBlendMode(render::getBlendMode(mat.blend));
        pipeline.setCullMode(render::getCullMode(mat.cull));

        mMaterialBuffer.upload(mat);
        mRenderableBuffer.upload(data, call);

        pipeline.bindTexture(0, mAssets.textureOrWhite(mat.albedo.texture));
        pipeline.bindTexture(1, mAssets.textureOrWhite(mat.emission.texture));
        pipeline.bindTexture(2, mAssets.textureOrWhite(mat.orm.texture));
        pipeline.bindTexture(3, mAssets.textureOrNormal(mat.normal.texture));

        pipeline.bindUniform(3, mRenderableBuffer.buffer());
        pipeline.bindUniform(4, mMaterialBuffer.buffer());

        call.draw(pipeline, data.instances(), data.instanceCount());
    }
}

const gpu::Texture& Scene::postSSAO(const gpu::Texture& source)
{
    // Right now SSAO is done in a simple way by directly darkening
    // the rendered scene, instead of being physically correct.
    // The proper way would be to run a depth pre-pass to get depth
    // and normals of opaque objects, compute SSAO, and apply it to
    // ambient light during the forward pass. But that makes things
    // more complicated for material shaders, which
    // aren’t in yet, and could hurt performance on mobile.
    // So for now we stick with this simpler version until it’s needed.

    gpu::Pipeline pipeline;

    /* --- Bind common stuff --- */

    pipeline.bindUniform(0, mFrustum.buffer());
    pipeline.bindUniform(1, mEnvironment.buffer());

    /* --- Generate ambient occlusion --- */

    pipeline.bindFramebuffer(mSwapAuxiliary.target());
    {
        pipeline.setViewport(mSwapAuxiliary.target());
        pipeline.useProgram(mPrograms.ssaoPass());

        pipeline.bindTexture(0, mTargetSceneDepth);
        pipeline.bindTexture(1, mTargetSceneNormal);
        pipeline.bindTexture(2, mAssets.textureSsaoKernel());
        pipeline.bindTexture(3, mAssets.textureSsaoNoise());

        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapAuxiliary.swap();

    /* --- Blur ambient occlusion --- */

    pipeline.useProgram(mPrograms.bilateralBlur());

    pipeline.bindTexture(1, mTargetSceneDepth);

    pipeline.bindFramebuffer(mSwapAuxiliary.target());
    {
        pipeline.bindTexture(0, mSwapAuxiliary.source());
        pipeline.setUniformFloat2(0, NX_VEC2(1.0f / mSwapAuxiliary.source().width(), 0.0f));
        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapAuxiliary.swap();

    pipeline.bindFramebuffer(mSwapAuxiliary.target());
    {
        pipeline.bindTexture(0, mSwapAuxiliary.source());
        pipeline.setUniformFloat2(0, NX_VEC2(0.0f, 1.0f / mSwapAuxiliary.source().height()));
        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapAuxiliary.swap();

    /* --- Apply SSAO --- */

    pipeline.bindFramebuffer(mSwapPostProcess.target());
    {
        pipeline.setViewport(mSwapPostProcess.target());
        pipeline.useProgram(mPrograms.ssaoPost());

        pipeline.bindTexture(0, source);
        pipeline.bindTexture(1, mSwapAuxiliary.source());

        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapPostProcess.swap();

    return mSwapPostProcess.source();
}

const gpu::Texture& Scene::postBloom(const gpu::Texture& source)
{
    gpu::Pipeline pipeline;

    /* --- Bind common stuff --- */

    pipeline.bindUniform(0, mEnvironment.buffer());

    /* --- Downsampling of the source --- */

    pipeline.useProgram(mPrograms.downsampling());
    pipeline.bindTexture(0, source);

    pipeline.setUniformFloat2(0, NX_IVec2Rcp(mTargetSceneColor.dimensions()));

    mMipChain.downsample(pipeline, 0, [&](int targetLevel, int sourceLevel) {
        pipeline.setUniformInt1(2, targetLevel);
        pipeline.draw(GL_TRIANGLES, 3);
        pipeline.setUniformFloat2(0, NX_IVec2Rcp(mMipChain.dimensions(targetLevel)));
        if (targetLevel == 0) {
            pipeline.bindTexture(0, mMipChain.texture());
        }
    });

    /* --- Apply bloom level factors --- */

    pipeline.useProgram(mPrograms.screenQuad());
    pipeline.setBlendMode(gpu::BlendMode::Multiply);

    mMipChain.iterate(pipeline, [&](int targetLevel) {
        pipeline.setUniformFloat4(0, NX_VEC4_1(mEnvironment.bloomLevels()[targetLevel]));
        pipeline.draw(GL_TRIANGLES, 3);
    });

    /* --- Upsampling of the source --- */

    pipeline.useProgram(mPrograms.upsampling());
    pipeline.setBlendMode(gpu::BlendMode::Additive);

    mMipChain.upsample(pipeline, [&](int targetLevel, int sourceLevel) {
        pipeline.draw(GL_TRIANGLES, 3);
    });

    pipeline.setBlendMode(gpu::BlendMode::Disabled);

    /* --- Applying bloom to the scene --- */

    pipeline.bindFramebuffer(mSwapPostProcess.target());
    pipeline.setViewport(mSwapPostProcess.target());

    pipeline.useProgram(mPrograms.bloomPost(mEnvironment.bloomMode()));

    pipeline.bindTexture(0, source);
    pipeline.bindTexture(1, mMipChain.texture());

    pipeline.draw(GL_TRIANGLES, 3);

    mSwapPostProcess.swap();

    return mSwapPostProcess.source();
}

void Scene::postFinal(const gpu::Texture& source)
{
    gpu::Pipeline pipeline;

    if (mTargetInfo.target != nullptr) {
        pipeline.bindFramebuffer(mTargetInfo.target->framebuffer());
    }
    pipeline.setViewport(mTargetInfo.resolution);

    pipeline.useProgram(mPrograms.output(mEnvironment.tonemapMode()));
    pipeline.bindUniform(0, mEnvironment.buffer());
    pipeline.bindTexture(0, source);

    pipeline.draw(GL_TRIANGLES, 3);
}

} // namespace scene
