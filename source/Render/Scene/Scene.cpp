/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "./Scene.hpp"

#include <Hyperion/HP_Render.h>

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

Scene::Scene(const render::SharedAssets& assets, HP_AppDesc& desc)
    : mAssetsCommon(assets)
    , mAssetsScene()
    , mLights(getResolution(desc.render3D.resolution), desc.render3D.shadowRes)
    , mPrograms(mAssetsCommon.vertexShaderScreen())
    , mFrustum()
{
    /* --- Tweak description --- */

    if (desc.render3D.resolution < HP_IVEC2_ONE) {
        desc.render3D.resolution = HP_GetDisplaySize();
    }

    desc.render3D.sampleCount = HP_MAX(desc.render3D.sampleCount, 1);

    /* --- Get internal resolution --- */

    mTargetInfo.resolution = desc.render3D.resolution;
    mTargetInfo.texelSize = HP_IVec2Rcp(desc.render3D.resolution);
    mTargetInfo.aspect = static_cast<float>(desc.render3D.resolution.x) / desc.render3D.resolution.y;

    /* --- Create render targets --- */

    mTargetSceneColor = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGBA16F,
            .data = nullptr,
            .width = int(mTargetInfo.resolution.x),
            .height = int(mTargetInfo.resolution.y),
            .depth = 0,
            .mipmap = false,
        }
    );

    mTargetSceneDepth = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_DEPTH_COMPONENT24,
            .data = nullptr,
            .width = int(mTargetInfo.resolution.x),
            .height = int(mTargetInfo.resolution.y),
            .depth = 0,
            .mipmap = false,
        }
    );

    /* --- Configure framebuffers --- */

    mFramebufferScene = gpu::Framebuffer({ &mTargetSceneColor }, &mTargetSceneDepth);
    if (desc.render3D.sampleCount > 1) {
        mFramebufferScene.setSampleCount(desc.render3D.sampleCount);
    }

    /* --- Create swap buffers --- */

    mSwapPostProcess = gpu::SwapBuffer(
        GL_RGB16F, desc.render3D.resolution.x, desc.render3D.resolution.y
    );

    /* --- Reserve enough space for the draw calls array --- */

    if (!mDrawCalls.reserve(1024)) {
        HP_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the draw call buffer");
    }

    if (!mDrawData.reserve(1024)) {
        HP_INTERNAL_LOG(W, "RENDER: Failed to pre-allocate the draw data buffer");
    }
}

/* === Private Implementation === */

void Scene::renderScene()
{
    gpu::Pipeline pipeline;

    /* --- Bind scene framebuffer, setup viewport, and clear --- */

    pipeline.bindFramebuffer(mFramebufferScene);
    pipeline.setViewport(mFramebufferScene);

    pipeline.setDepthMode(gpu::DepthMode::WriteOnly);
    pipeline.clear(mFramebufferScene, mEnvironment.background);

    /* --- Bind common UBOs --- */

    pipeline.bindUniform(0, mFrustum.buffer());

    /* --- Render skybox --- */

    if (mEnvironment.sky.cubemap != nullptr)
    {
        pipeline.setDepthMode(gpu::DepthMode::Disabled);
        pipeline.useProgram(mPrograms.skybox());

        pipeline.bindTexture(0, mEnvironment.sky.cubemap->texture());
        pipeline.setUniformFloat4(0, mEnvironment.sky.rotation);
        pipeline.setUniformFloat1(1, mEnvironment.sky.intensity);

        pipeline.draw(GL_TRIANGLES, 36);
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
    pipeline.setUniformUint2(11, mFramebufferScene.dimension());
    pipeline.setUniformUint3(12, mLights.clusterCount());
    pipeline.setUniformUint1(13, mLights.maxLightsPerCluster());
    pipeline.setUniformFloat1(14, mLights.clusterSliceScale());
    pipeline.setUniformFloat1(15, mLights.clusterSliceBias());

    pipeline.setUniformFloat1(19, mEnvironment.sky.diffuse * mEnvironment.sky.intensity);
    pipeline.setUniformFloat1(20, mEnvironment.sky.specular * mEnvironment.sky.intensity);

    if (mEnvironment.sky.probe != nullptr) {
        pipeline.setUniformInt1(17, true);
        pipeline.setUniformFloat4(18, mEnvironment.sky.rotation);
        pipeline.setUniformInt1(21, mEnvironment.sky.probe->prefilter().mipLevels());
    }
    else {
        pipeline.setUniformFloat3(16, mEnvironment.ambient);
        pipeline.setUniformInt1(17, false);
    }

    /* --- Bind constant textures --- */

    pipeline.bindTexture(4, mAssetsScene.textureBrdfLut());
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
            pipeline.setBlendMode(gpu::BlendMode::Additive);
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

        auto texOr = [](const HP_Texture* tex, const gpu::Texture& def) -> const gpu::Texture& {
            return tex ? tex->gpuTexture() : def;
        };

        pipeline.bindTexture(0, texOr(mat.albedo.texture,   mAssetsCommon.textureWhite().gpuTexture()));
        pipeline.bindTexture(1, texOr(mat.emission.texture, mAssetsCommon.textureWhite().gpuTexture()));
        pipeline.bindTexture(2, texOr(mat.orm.texture,      mAssetsCommon.textureWhite().gpuTexture()));
        pipeline.bindTexture(3, texOr(mat.normal.texture,   mAssetsScene.textureNormal()));

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

        pipeline.setUniformFloat3(22, mat.emission.color);
        pipeline.setUniformFloat1(23, mat.emission.energy);
        pipeline.setUniformFloat1(24, mat.orm.aoLightAffect);
        pipeline.setUniformFloat1(25, mat.orm.occlusion);
        pipeline.setUniformFloat1(26, mat.orm.roughness);
        pipeline.setUniformFloat1(27, mat.orm.metalness);
        pipeline.setUniformFloat1(28, mat.normal.scale);
        pipeline.setUniformFloat1(29, mat.alphaCutOff);
        pipeline.setUniformUint1(30, call.mesh().layerMask);

        /* --- Draw! --- */

        call.draw(pipeline, data.instances(), data.instanceCount());
    }

    /* --- Resolve in case of multi sampled scene --- */

    mFramebufferScene.resolve();
}

void Scene::postProcess()
{
    gpu::Pipeline pipeline;

    pipeline.useProgram(mPrograms.output(mEnvironment.tonemap.mode));
    pipeline.bindTexture(0, mTargetSceneColor);
    pipeline.setViewport(HP_GetWindowSize());

    pipeline.setUniformFloat1(0, mEnvironment.tonemap.exposure);
    pipeline.setUniformFloat1(1, mEnvironment.tonemap.white);
    pipeline.setUniformFloat1(2, mEnvironment.adjustment.brightness);
    pipeline.setUniformFloat1(3, mEnvironment.adjustment.contrast);
    pipeline.setUniformFloat1(4, mEnvironment.adjustment.saturation);

    pipeline.draw(GL_TRIANGLES, 3);
}

} // namespace scene
