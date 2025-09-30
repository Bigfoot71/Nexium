/* Scene.cpp -- Scene system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./Scene.hpp"

#include <Hyperion/HP_Render.h>

#include "../../Detail/BuildInfo.hpp"
#include "../HP_ReflectionProbe.hpp"
#include "../HP_Cubemap.hpp"
#include "../HP_Texture.hpp"

/* === Helper Functions === */

namespace {
// TODO: Review the lights init which needs the res before it is defined...
HP_IVec2 getResolution(HP_IVec2 res) {
    return (res > HP_IVEC2_ZERO) ? res : HP_GetDisplaySize();
}
} // namespace

namespace scene {

/* === Public Implementation === */

Scene::Scene(render::ProgramCache& programs, render::AssetCache& assets, HP_AppDesc& desc)
    : mPrograms(programs)
    , mAssets(assets)
    , mLights(programs, assets, getResolution(desc.render3D.resolution), desc.render3D.shadowRes)
    , mFrustum()
{
    /* --- Tweak description --- */

    if (desc.render3D.resolution < HP_IVEC2_ONE) {
        desc.render3D.resolution = HP_GetDisplaySize();
    }

    desc.render3D.sampleCount = HP_MAX(desc.render3D.sampleCount, 1);

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
            .internalFormat = GL_RG16F,
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
        HP_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the draw call buffer");
    }

    if (!mDrawData.reserve(1024)) {
        HP_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the draw data buffer");
    }
}

void Scene::begin(const HP_Camera& camera, const HP_Environment& env, const HP_RenderTexture* target)
{
    mTargetInfo.target = target;
    mTargetInfo.resolution = (target) ? target->framebuffer().dimensions() : HP_GetWindowSize();
    mTargetInfo.aspect = static_cast<float>(mTargetInfo.resolution.x) / mTargetInfo.resolution.y;

    mFrustum.update(camera, mTargetInfo.aspect);
    mEnvironment = env;
}

void Scene::end()
{
    /* --- Sort draw calls --- */

    mDrawCalls.sort(DrawCall::Category::TRANSPARENT,
        [this](const DrawCall& a, const DrawCall& b) {
            float maxDistA = mFrustum.getDistanceSquaredToFarthestPoint(a.mesh().aabb, mDrawData[a.dataIndex()].matrix());
            float maxDistB = mFrustum.getDistanceSquaredToFarthestPoint(b.mesh().aabb, mDrawData[b.dataIndex()].matrix());
            return maxDistA > maxDistB;
        }
    );

    /* --- Process lights --- */

    mLights.process({
        .viewFrustum = mFrustum,
        .environement = mEnvironment,
        .boneBuffer = mBoneBuffer,
        .drawCalls = mDrawCalls,
        .drawData = mDrawData
    });

    /* --- Render scene --- */

    renderScene();

    const gpu::Texture* source = &mTargetSceneColor;

    if (mEnvironment.ssao.enabled) {
        source = &postSSAO(*source);
    }

    if (mEnvironment.bloom.mode != HP_BLOOM_DISABLED) {
        source = &postBloom(*source);
    }

    postFinal(*source);

    /* --- Reset state --- */

    mBoneBuffer.clear();
    mDrawCalls.clear();
    mDrawData.clear();
}

/* === Private Implementation === */

void Scene::renderScene()
{
    gpu::Pipeline pipeline;

    /* --- Calculate background color --- */

    bool hasFog = (mEnvironment.fog.mode != HP_FOG_DISABLED);
    HP_Color background = mEnvironment.background;

    if (hasFog) {
        background = HP_ColorLerp(
            background,
            mEnvironment.fog.color,
            mEnvironment.fog.skyAffect
        );
    }

    /* --- Bind scene framebuffer, setup viewport, and clear --- */

    pipeline.bindFramebuffer(mFramebufferScene);
    pipeline.setViewport(mFramebufferScene);

    pipeline.setDepthMode(gpu::DepthMode::WriteOnly);

    pipeline.clearDepth(1.0f);
    pipeline.clearColor(0, background);
    pipeline.clearColor(1, HP_COLOR(0.25f, 0.25f, 1.0f, 1.0f));

    /* --- Bind common UBOs --- */

    pipeline.bindUniform(0, mFrustum.buffer());

    /* --- Render skybox --- */

    if (mEnvironment.sky.cubemap != nullptr)
    {
        mFramebufferScene.setDrawBuffers({0});

        pipeline.setDepthMode(gpu::DepthMode::Disabled);
        pipeline.useProgram(mPrograms.skybox());

        pipeline.bindTexture(0, mEnvironment.sky.cubemap->texture());
        pipeline.setUniformFloat4(0, mEnvironment.sky.rotation);
        pipeline.setUniformFloat1(1, mEnvironment.sky.intensity);
        pipeline.setUniformFloat1(2, hasFog ? mEnvironment.fog.skyAffect : 0.0f);
        pipeline.setUniformFloat3(3, mEnvironment.fog.color);

        pipeline.draw(GL_TRIANGLES, 36);

        mFramebufferScene.enableAllDrawBuffers();
    }

    /* --- Setup forward pipeline --- */

    pipeline.setDepthMode(gpu::DepthMode::TestAndWrite);
    pipeline.useProgram(mPrograms.forward());

    /* --- Bind lighting related SSBOs --- */

    pipeline.bindStorage(0, mLights.lightsBuffer());
    pipeline.bindStorage(1, mLights.shadowBuffer());
    pipeline.bindStorage(2, mLights.tilesBuffer());
    pipeline.bindStorage(3, mLights.indexBuffer());
    pipeline.bindStorage(4, mBoneBuffer.buffer());

    /* --- Send constant uniforms --- */

    pipeline.setUniformInt1(10, mLights.activeCount() > 0);
    pipeline.setUniformUint2(11, mFramebufferScene.dimensions());
    pipeline.setUniformUint3(12, mLights.clusterCount());
    pipeline.setUniformUint1(13, mLights.maxLightsPerCluster());
    pipeline.setUniformFloat1(14, mLights.clusterSliceScale());
    pipeline.setUniformFloat1(15, mLights.clusterSliceBias());

    pipeline.setUniformFloat1(19, mEnvironment.sky.diffuse * mEnvironment.sky.intensity);
    pipeline.setUniformFloat1(20, mEnvironment.sky.specular * mEnvironment.sky.intensity);

    if (mEnvironment.sky.probe != nullptr) {
        pipeline.setUniformInt1(17, true);
        pipeline.setUniformFloat4(18, mEnvironment.sky.rotation);
        pipeline.setUniformInt1(21, mEnvironment.sky.probe->prefilter().numLevels());
    }
    else {
        pipeline.setUniformFloat3(16, mEnvironment.ambient);
        pipeline.setUniformInt1(17, false);
    }

    if (hasFog) {
        pipeline.setUniformFloat1(22, mEnvironment.fog.skyAffect);
        pipeline.setUniformFloat1(23, mEnvironment.fog.density);
        if (mEnvironment.fog.mode == HP_FOG_LINEAR) {
            pipeline.setUniformFloat1(24, mEnvironment.fog.start);
            pipeline.setUniformFloat1(25, mEnvironment.fog.end);
        }
        pipeline.setUniformFloat3(26, mEnvironment.fog.color);
    }
    else {
        pipeline.setUniformFloat1(22, 0.0f);
    }

    pipeline.setUniformInt1(27, mEnvironment.fog.mode);

    /* --- Bind constant textures --- */

    pipeline.bindTexture(4, mAssets.textureBrdfLut());
    pipeline.bindTexture(7, mLights.shadowCube());
    pipeline.bindTexture(8, mLights.shadow2D());

    if (mEnvironment.sky.probe != nullptr) {
        pipeline.bindTexture(5, mEnvironment.sky.probe->irradiance());
        pipeline.bindTexture(6, mEnvironment.sky.probe->prefilter());
    }

    /* --- Ensures SSBOs are ready (especially clusters) --- */

    pipeline.memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /* --- Render objects --- */

    for (const DrawCall& call : mDrawCalls.categories(DrawCall::OPAQUE, DrawCall::TRANSPARENT))
    {
        /* --- Apply layer and frustum culling --- */

        if ((mFrustum.cullMask() & call.mesh().layerMask) == 0) {
            continue;
        }

        const DrawData& data = mDrawData[call.dataIndex()];
        bool useInstancing = data.useInstancing();

        if (!useInstancing && !mFrustum.containsObb(call.mesh().aabb, data.transform())) {
            continue;
        }

        /* --- Set pipeline state --- */

        const HP_Material& mat = call.material();

        switch (mat.blend) {
        case HP_BLEND_OPAQUE:
            pipeline.setBlendMode(gpu::BlendMode::Disabled);
            break;
        case HP_BLEND_ALPHA:
            pipeline.setBlendMode(gpu::BlendMode::Alpha);
            break;
        case HP_BLEND_ADD:
            pipeline.setBlendMode(gpu::BlendMode::AddAlpha);
            break;
        case HP_BLEND_MUL:
            pipeline.setBlendMode(gpu::BlendMode::Multiply);
            break;
        }

        switch (mat.cull) {
        case HP_CULL_NONE:
            pipeline.setCullMode(gpu::CullMode::Disabled);
            break;
        case HP_CULL_BACK:
            pipeline.setCullMode(gpu::CullMode::Back);
            break;
        case HP_CULL_FRONT:
            pipeline.setCullMode(gpu::CullMode::Front);
            break;
        }

        /* --- Bind textures --- */

        pipeline.bindTexture(0, mAssets.textureOrWhite(mat.albedo.texture));
        pipeline.bindTexture(1, mAssets.textureOrWhite(mat.emission.texture));
        pipeline.bindTexture(2, mAssets.textureOrWhite(mat.orm.texture));
        pipeline.bindTexture(3, mAssets.textureOrNormal(mat.normal.texture));

        /* --- Send matrices --- */

        pipeline.setUniformMat4(0, data.matrix());
        pipeline.setUniformMat3(1, data.normal());

        /* --- Send animation data --- */

        pipeline.setUniformInt1(5, data.isAnimated());
        pipeline.setUniformInt1(6, data.boneMatrixOffset());

        /* --- Send instance data --- */

        pipeline.setUniformInt1(7, useInstancing);

        /* --- Send material data --- */

        pipeline.setUniformFloat4(2, mat.albedo.color);
        pipeline.setUniformFloat2(3, mat.texOffset);
        pipeline.setUniformFloat2(4, mat.texScale);
        pipeline.setUniformUint1(8, mat.billboard);

        pipeline.setUniformFloat3(28, mat.emission.color);
        pipeline.setUniformFloat1(29, mat.emission.energy);
        pipeline.setUniformFloat1(30, mat.orm.aoLightAffect);
        pipeline.setUniformFloat1(31, mat.orm.occlusion);
        pipeline.setUniformFloat1(32, mat.orm.roughness);
        pipeline.setUniformFloat1(33, mat.orm.metalness);
        pipeline.setUniformFloat1(34, mat.normal.scale);
        pipeline.setUniformFloat1(35, mat.alphaCutOff);
        pipeline.setUniformUint1(36, call.mesh().layerMask);

        /* --- Draw! --- */

        call.draw(pipeline, data.instances(), data.instanceCount());
    }

    /* --- Resolve in case of multi sampled scene --- */

    mFramebufferScene.resolve();
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

    /* --- Generate ambient occlusion --- */

    pipeline.bindFramebuffer(mSwapAuxiliary.target());
    {
        pipeline.setViewport(mSwapAuxiliary.target());
        pipeline.useProgram(mPrograms.ssaoPass());

        pipeline.bindTexture(0, mTargetSceneDepth);
        pipeline.bindTexture(1, mTargetSceneNormal);
        pipeline.bindTexture(2, mAssets.textureSsaoKernel());
        pipeline.bindTexture(3, mAssets.textureSsaoNoise());

        pipeline.setUniformFloat1(0, mEnvironment.ssao.radius);
        pipeline.setUniformFloat1(1, mEnvironment.ssao.bias);

        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapAuxiliary.swap();

    /* --- Blur ambient occlusion --- */

    pipeline.useProgram(mPrograms.bilateralBlur());

    pipeline.bindTexture(1, mTargetSceneDepth);
    pipeline.setUniformFloat1(1, mEnvironment.ssao.radius);

    pipeline.bindFramebuffer(mSwapAuxiliary.target());
    {
        pipeline.bindTexture(0, mSwapAuxiliary.source());
        pipeline.setUniformFloat2(0, HP_VEC2(1.0f / mSwapAuxiliary.source().width(), 0.0f));
        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapAuxiliary.swap();

    pipeline.bindFramebuffer(mSwapAuxiliary.target());
    {
        pipeline.bindTexture(0, mSwapAuxiliary.source());
        pipeline.setUniformFloat2(0, HP_VEC2(0.0f, 1.0f / mSwapAuxiliary.source().height()));
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

        pipeline.setUniformFloat1(0, mEnvironment.ssao.intensity);
        pipeline.setUniformFloat1(1, mEnvironment.ssao.power);

        pipeline.draw(GL_TRIANGLES, 3);
    }
    mSwapPostProcess.swap();

    return mSwapPostProcess.source();
}

const gpu::Texture& Scene::postBloom(const gpu::Texture& source)
{
    // TODO: Add control over the number of levels used for bloom calculation

    gpu::Pipeline pipeline;

    /* --- Calculation of bloom parameters --- */

    float knee = mEnvironment.bloom.threshold * mEnvironment.bloom.softThreshold;

    HP_Vec4 prefilter;
    prefilter.x = mEnvironment.bloom.threshold;
    prefilter.y = mEnvironment.bloom.threshold - knee;
    prefilter.z = 2.0f * knee;
    prefilter.w = 0.25f / (knee + 1e-6f);

    /* --- Downsampling of the source --- */

    pipeline.useProgram(mPrograms.downsampling());
    pipeline.bindTexture(0, source);

    pipeline.setUniformFloat2(0, HP_IVec2Rcp(mTargetSceneColor.dimensions()));
    pipeline.setUniformFloat4(1, prefilter);

    mMipChain.downsample(pipeline, 0, [&](uint32_t targetLevel, uint32_t sourceLevel) {
        pipeline.setUniformInt1(2, targetLevel);
        if (targetLevel > 0) {
            mMipChain.setMipLevelRange(sourceLevel, sourceLevel);
        }
        pipeline.draw(GL_TRIANGLES, 3);
        pipeline.setUniformFloat2(0, HP_IVec2Rcp(mMipChain.dimensions(targetLevel)));
        if (targetLevel == 0) {
            pipeline.bindTexture(0, mMipChain.texture());
        }
    });

    /* --- Upsampling of the source --- */

    pipeline.useProgram(mPrograms.upsampling());
    pipeline.setBlendMode(gpu::BlendMode::Additive);

    mMipChain.upsample(pipeline, [&](uint32_t targetLevel, uint32_t sourceLevel) {
        pipeline.setUniformFloat2(0, mEnvironment.bloom.filterRadius * HP_IVec2Rcp(mMipChain.dimensions(sourceLevel)));
        mMipChain.setMipLevelRange(sourceLevel, sourceLevel);
        pipeline.draw(GL_TRIANGLES, 3);
    });

    pipeline.setBlendMode(gpu::BlendMode::Disabled);

    /* --- Reset mipmap sampling levels for debugging with RenderDoc */

    if constexpr (detail::BuildInfo::debug) {
        mMipChain.setMipLevelRange(0, mMipChain.numLevels() - 1);
    }

    /* --- Applying bloom to the scene --- */

    pipeline.bindFramebuffer(mSwapPostProcess.target());
    pipeline.setViewport(mSwapPostProcess.target());

    pipeline.useProgram(mPrograms.bloomPost(mEnvironment.bloom.mode));
    pipeline.setUniformFloat1(0, mEnvironment.bloom.strength);

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

    pipeline.useProgram(mPrograms.output(mEnvironment.tonemap.mode));
    pipeline.bindTexture(0, source);

    pipeline.setUniformFloat1(0, mEnvironment.tonemap.exposure);
    pipeline.setUniformFloat1(1, mEnvironment.tonemap.white);
    pipeline.setUniformFloat1(2, mEnvironment.adjustment.brightness);
    pipeline.setUniformFloat1(3, mEnvironment.adjustment.contrast);
    pipeline.setUniformFloat1(4, mEnvironment.adjustment.saturation);

    pipeline.draw(GL_TRIANGLES, 3);
}

} // namespace scene
