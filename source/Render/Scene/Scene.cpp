/* Scene.cpp -- Scene system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./Scene.hpp"

#include <NX/NX_Runtime.h>
#include <NX/NX_Display.h>
#include <NX/NX_Window.h>
#include <NX/NX_Render.h>

#include "../../Detail/BuildInfo.hpp"
#include "../../NX_RenderTexture.hpp"
#include "../../INX_PoolAssets.hpp"
#include "../../NX_Texture.hpp"

#include "../NX_ReflectionProbe.hpp"
#include "../NX_Cubemap.hpp"
#include "NX/NX_Shader3D.h"

namespace scene {

/* === Public Implementation === */

Scene::Scene(render::ProgramCache& programs, NX_AppDesc& desc)
    : mPrograms(programs)
    , mDrawCalls(1024), mLights(programs, desc), mFrustum()
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
}

void Scene::begin(const NX_Camera& camera, const NX_Environment& env, const NX_RenderTexture* target)
{
    /* --- Get target (where we blit) info --- */

    mTargetInfo.target = target;
    mTargetInfo.resolution = (target) ? NX_GetRenderTextureSize(target) : NX_GetWindowSize();
    mTargetInfo.aspect = static_cast<float>(mTargetInfo.resolution.x) / mTargetInfo.resolution.y;

    /* --- Update managers --- */

    mFrustum.update(camera, mTargetInfo.aspect);
    mEnvironment.update(env, mMipChain.numLevels());
}

void Scene::end()
{
    /* --- Upload draw calls data --- */

    mDrawCalls.upload();

    /* --- Process lights --- */

    mLights.process({
        .viewFrustum = mFrustum,
        .environment = mEnvironment,
        .drawCalls = mDrawCalls
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

    /* --- View layer/furstum culling and sorting --- */

    mDrawCalls.culling(mFrustum, mFrustum.cullMask());
    mDrawCalls.sorting(mFrustum, mEnvironment);

    /* --- Render scene --- */

    gpu::Pipeline([this](const gpu::Pipeline& pipeline) { // NOLINT
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

    // REVIEW: We can collect the used programs rather than iterating over all programs
    INX_Pool.ForEach<NX_Shader3D>([](NX_Shader3D& shader) {
        shader.ClearDynamicBuffer();
    });

    /* --- Reset state --- */

    mDrawCalls.clear();
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

    mFramebufferScene.enableDrawBuffers();
}

void Scene::renderPrePass(const gpu::Pipeline& pipeline)
{
    if (mDrawCalls.uniqueVisible().category(DRAW_PREPASS).empty()) {
        return;
    }

    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.setColorWrite(gpu::ColorWrite::Disabled);

    pipeline.bindStorage(0, mDrawCalls.sharedBuffer());
    pipeline.bindStorage(1, mDrawCalls.uniqueBuffer());
    pipeline.bindStorage(2, mDrawCalls.boneBuffer());

    pipeline.bindUniform(0, mFrameUniform);
    pipeline.bindUniform(1, mFrustum.buffer());
    pipeline.bindUniform(2, mEnvironment.buffer());

    for (int uniqueIndex : mDrawCalls.uniqueVisible().category(DRAW_PREPASS))
    {
        const DrawUnique& unique = mDrawCalls.uniqueData()[uniqueIndex];
        const NX_Material& mat = unique.material;

        const NX_Shader3D* shader = INX_Assets.Select(mat.shader, INX_Shader3DAsset::DEFAULT);
        pipeline.useProgram(shader->GetProgram(NX_Shader3D::Variant::SCENE_PREPASS));

        pipeline.setDepthFunc(gpu::getDepthFunc(mat.depth.test));
        pipeline.setCullMode(gpu::getCullMode(mat.cull));

        shader->BindTextures(pipeline, unique.textures);
        shader->BindUniforms(pipeline, unique.dynamicRangeIndex);

        const NX_Texture* texAlbedo = INX_Assets.Select(
            unique.material.albedo.texture,
            INX_TextureAsset::WHITE
        );

        pipeline.bindTexture(0, texAlbedo->gpu);

        pipeline.setUniformUint1(0, unique.sharedDataIndex);
        pipeline.setUniformUint1(1, unique.uniqueDataIndex);

        mDrawCalls.draw(pipeline, unique);
    }
}

void Scene::renderScene(const gpu::Pipeline& pipeline)
{
    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.setColorWrite(gpu::ColorWrite::RGBA);

    pipeline.bindStorage(0, mDrawCalls.sharedBuffer());
    pipeline.bindStorage(1, mDrawCalls.uniqueBuffer());
    pipeline.bindStorage(2, mDrawCalls.boneBuffer());
    pipeline.bindStorage(3, mLights.lightsBuffer());
    pipeline.bindStorage(4, mLights.shadowBuffer());
    pipeline.bindStorage(5, mLights.tilesBuffer());
    pipeline.bindStorage(6, mLights.indexBuffer());

    pipeline.bindTexture(4, INX_Assets.Get(INX_TextureAsset::BRDF_LUT)->gpu);
    pipeline.bindTexture(7, mLights.shadowMap(NX_LIGHT_DIR));
    pipeline.bindTexture(8, mLights.shadowMap(NX_LIGHT_SPOT));
    pipeline.bindTexture(9, mLights.shadowMap(NX_LIGHT_OMNI));

    pipeline.bindUniform(0, mFrameUniform);
    pipeline.bindUniform(1, mFrustum.buffer());
    pipeline.bindUniform(2, mEnvironment.buffer());

    if (mEnvironment.skyProbe() != nullptr) {
        pipeline.bindTexture(5, mEnvironment.skyProbe()->irradiance());
        pipeline.bindTexture(6, mEnvironment.skyProbe()->prefilter());
    }

    // Ensures SSBOs are ready (especially clusters)
    pipeline.memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    for (int uniqueIndex : mDrawCalls.uniqueVisible().categories(DRAW_OPAQUE, DRAW_PREPASS, DRAW_TRANSPARENT))
    {
        const DrawUnique& unique = mDrawCalls.uniqueData()[uniqueIndex];
        const NX_Material& mat = unique.material;

        const NX_Shader3D* shader = INX_Assets.Select(mat.shader, INX_Shader3DAsset::DEFAULT);
        pipeline.useProgram(shader->GetProgramFromShadingMode(unique.material.shading));

        shader->BindTextures(pipeline, unique.textures);
        shader->BindUniforms(pipeline, unique.dynamicRangeIndex);

        pipeline.setDepthFunc(mat.depth.prePass ? gpu::DepthFunc::Equal : gpu::getDepthFunc(mat.depth.test));
        pipeline.setBlendMode(gpu::getBlendMode(mat.blend));
        pipeline.setCullMode(gpu::getCullMode(mat.cull));

        pipeline.bindTexture(0, INX_Assets.Select(mat.albedo.texture, INX_TextureAsset::WHITE)->gpu);
        pipeline.bindTexture(1, INX_Assets.Select(mat.emission.texture, INX_TextureAsset::WHITE)->gpu);
        pipeline.bindTexture(2, INX_Assets.Select(mat.orm.texture, INX_TextureAsset::WHITE)->gpu);
        pipeline.bindTexture(3, INX_Assets.Select(mat.normal.texture, INX_TextureAsset::NORMAL)->gpu);

        pipeline.setUniformUint1(0, unique.sharedDataIndex);
        pipeline.setUniformUint1(1, unique.uniqueDataIndex);

        mDrawCalls.draw(pipeline, unique);
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
        pipeline.bindTexture(2, INX_Assets.Get(INX_TextureAsset::SSAO_KERNEL)->gpu);
        pipeline.bindTexture(3, INX_Assets.Get(INX_TextureAsset::SSAO_NOISE)->gpu);

        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapAuxiliary.swap();

    /* --- Blur ambient occlusion --- */

    pipeline.useProgram(mPrograms.ssaoBilateralBlur());

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

    mMipChain.downsample(pipeline, 0, [&](int targetLevel, int sourceLevel) {
        const gpu::Texture& texSource = (targetLevel == 0) ? source : mMipChain.texture();
        pipeline.setUniformFloat2(0, NX_IVec2Rcp(texSource.dimensions()));
        pipeline.setUniformInt1(1, targetLevel);
        pipeline.bindTexture(0, texSource);
        pipeline.draw(GL_TRIANGLES, 3);
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
        pipeline.bindFramebuffer(mTargetInfo.target->gpu);
    }
    pipeline.setViewport(mTargetInfo.resolution);

    pipeline.useProgram(mPrograms.output(mEnvironment.tonemapMode()));
    pipeline.bindUniform(0, mEnvironment.buffer());
    pipeline.bindTexture(0, source);

    pipeline.draw(GL_TRIANGLES, 3);
}

} // namespace scene
