/* ProgramCache.cpp -- Manage internal GPU program storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./ProgramCache.hpp"
#include "Hyperion/HP_Render.h"

#include <shaders/screen.vert.h>
#include <shaders/cube.vert.h>

#include <shaders/cubemap_from_equirectangular.frag.h>
#include <shaders/cubemap_irradiance.frag.h>
#include <shaders/cubemap_prefilter.frag.h>
#include <shaders/cubemap_skybox.frag.h>

#include <shaders/light_culling.comp.h>
#include <shaders/forward.vert.h>
#include <shaders/forward.frag.h>
#include <shaders/skybox.vert.h>
#include <shaders/skybox.frag.h>
#include <shaders/shadow.vert.h>
#include <shaders/shadow.frag.h>

#include <shaders/bilateral_blur.frag.h>
#include <shaders/downsampling.frag.h>
#include <shaders/upsampling.frag.h>
#include <shaders/bloom_post.frag.h>
#include <shaders/ssao_pass.frag.h>
#include <shaders/ssao_post.frag.h>
#include <shaders/output.frag.h>

#include <shaders/generic.vert.h>
#include <shaders/generic.frag.h>
#include <shaders/overlay.frag.h>

namespace render {

/* === Public Implementation === */

ProgramCache::ProgramCache()
    : mVertexShaderOverlayGeneric(GL_VERTEX_SHADER, GENERIC_VERT)
    , mVertexShaderScreen(GL_VERTEX_SHADER, SCREEN_VERT)
    , mVertexShaderCube(GL_VERTEX_SHADER, CUBE_VERT)
{ }

gpu::Program& ProgramCache::cubemapFromEquirectangular()
{
    if (mCubemapFromEquirectangular.isValid()) {
        return mCubemapFromEquirectangular;
    }

    mCubemapFromEquirectangular = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(GL_FRAGMENT_SHADER, CUBEMAP_FROM_EQUIRECTANGULAR_FRAG)
    );

    return mCubemapFromEquirectangular;
}

gpu::Program& ProgramCache::cubemapIrradiance()
{
    if (mCubemapIrradiance.isValid()) {
        return mCubemapIrradiance;
    }

    mCubemapIrradiance = gpu::Program(
        mVertexShaderCube,
        gpu::Shader(GL_FRAGMENT_SHADER, CUBEMAP_IRRADIANCE_FRAG)
    );

    return mCubemapIrradiance;
}

gpu::Program& ProgramCache::cubemapPrefilter()
{
    if (mCubemapPrefilter.isValid()) {
        return mCubemapPrefilter;
    }

    mCubemapPrefilter = gpu::Program(
        mVertexShaderCube,
        gpu::Shader(GL_FRAGMENT_SHADER, CUBEMAP_PREFILTER_FRAG)
    );

    return mCubemapPrefilter;
}

gpu::Program& ProgramCache::cubemapSkybox()
{
    if (mCubemapSkybox.isValid()) {
        return mCubemapSkybox;
    }

    mCubemapSkybox = gpu::Program(
        mVertexShaderCube,
        gpu::Shader(GL_FRAGMENT_SHADER, CUBEMAP_SKYBOX_FRAG)
    );

    return mCubemapSkybox;
}

gpu::Program& ProgramCache::lightCulling()
{
    if (mLightCulling.isValid()) {
        return mLightCulling;
    }

    mLightCulling = gpu::Program(
        gpu::Shader(GL_COMPUTE_SHADER, LIGHT_CULLING_COMP)
    );

    return mLightCulling;
}

gpu::Program& ProgramCache::forward()
{
    if (mForward.isValid()) {
        return mForward;
    }

    mForward = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, FORWARD_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, FORWARD_FRAG)
    );

    return mForward;
}

gpu::Program& ProgramCache::skybox()
{
    if (mSkybox.isValid()) {
        return mSkybox;
    }

    mSkybox = gpu::Program(
        gpu::Shader(
            GL_VERTEX_SHADER,
            SKYBOX_VERT
        ),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            SKYBOX_FRAG
        )
    );

    return mSkybox;
}

gpu::Program& ProgramCache::shadow()
{
    if (mShadow.isValid()) {
        return mShadow;
    }

    mShadow = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, SHADOW_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, SHADOW_FRAG)
    );

    return mShadow;
}

gpu::Program& ProgramCache::output(HP_Tonemap tonemap)
{
    if (mOutput[tonemap].isValid()) {
        return mOutput[tonemap];
    }

    const char* tonemapper = "TONEMAPPER TONEMAP_LINEAR";

    switch (tonemap) {
    case HP_TONEMAP_LINEAR:
        break;
    case HP_TONEMAP_REINHARD:
        tonemapper = "TONEMAPPER TONEMAP_REINHARD";
        break;
    case HP_TONEMAP_FILMIC:
        tonemapper = "TONEMAPPER TONEMAP_FILMIC";
        break;
    case HP_TONEMAP_ACES:
        tonemapper = "TONEMAPPER TONEMAP_ACES";
        break;
    case HP_TONEMAP_AGX:
        tonemapper = "TONEMAPPER TONEMAP_AGX";
        break;
    default:
        HP_INTERNAL_LOG(W, "RENDER: Unknown tonemap mode (%i); Linear will be used", tonemap);
        break;
    }

    gpu::Shader frag(GL_FRAGMENT_SHADER, OUTPUT_FRAG, {tonemapper});
    mOutput[tonemap] = gpu::Program(mVertexShaderScreen, frag);

    return mOutput[tonemap];
}

gpu::Program& ProgramCache::bilateralBlur()
{
    if (mBilateralBlur.isValid()) {
        return mBilateralBlur;
    }

    mBilateralBlur = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            BILATERAL_BLUR_FRAG
        )
    );

    return mBilateralBlur;
}

gpu::Program& ProgramCache::downsampling()
{
    if (mDownsampling.isValid()) {
        return mDownsampling;
    }

    mDownsampling = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(GL_FRAGMENT_SHADER, DOWNSAMPLING_FRAG)
    );

    return mDownsampling;
}

gpu::Program& ProgramCache::upsampling()
{
    if (mUpsampling.isValid()) {
        return mUpsampling;
    }

    mUpsampling = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(GL_FRAGMENT_SHADER, UPSAMPLING_FRAG)
    );

    return mUpsampling;
}

gpu::Program& ProgramCache::bloomPost(HP_BloomMode mode)
{
    SDL_assert(mode != HP_BLOOM_DISABLED);

    if (mBloomPost[mode].isValid()) {
        return mBloomPost[mode];
    }

    const char* bloomMode = "BLOOM_MIX";

    switch (mode) {
    case HP_BLOOM_MIX:
        break;
    case HP_BLOOM_ADDITIVE:
        bloomMode = "BLOOM_ADDITIVE";
        break;
    case HP_BLOOM_SCREEN:
        bloomMode = "BLOOM_SCREEN";
        break;
    default:
        HP_INTERNAL_LOG(W, "RENDER: Unknown bloom mode (%i); Mix will be used", mode);
        break;
    }

    mBloomPost[mode] = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(GL_FRAGMENT_SHADER, BLOOM_POST_FRAG, {bloomMode})
    );

    return mBloomPost[mode];
}

gpu::Program& ProgramCache::ssaoPass()
{
    if (mSsaoPass.isValid()) {
        return mSsaoPass;
    }

    mSsaoPass = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            SSAO_PASS_FRAG
        )
    );

    return mSsaoPass;
}

gpu::Program& ProgramCache::ssaoPost()
{
    if (mSsaoPost.isValid()) {
        return mSsaoPost;
    }

    mSsaoPost = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            SSAO_POST_FRAG
        )
    );

    return mSsaoPost;
}

gpu::Program& ProgramCache::overlayFontBitmap()
{
    if (mOverlayFontBitmap.isValid()) {
        return mOverlayFontBitmap;
    }

    mOverlayFontBitmap = gpu::Program(
        mVertexShaderOverlayGeneric,
        gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"FONT_BITMAP"})
    );

    return mOverlayFontBitmap;
}

gpu::Program& ProgramCache::overlayFontSDF()
{
    if (mOverlayFontSDF.isValid()) {
        return mOverlayFontSDF;
    }
    
    mOverlayFontSDF = gpu::Program(
        mVertexShaderOverlayGeneric,
        gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"FONT_SDF"})
    );

    return mOverlayFontSDF;
}

gpu::Program& ProgramCache::overlayTexture()
{
    if (mOverlayTexture.isValid()) {
        return mOverlayTexture;
    }

    mOverlayTexture = gpu::Program(
        mVertexShaderOverlayGeneric,
        gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"TEXTURE"})
    );

    return mOverlayTexture;
}

gpu::Program& ProgramCache::overlayColor()
{
    if (mOverlayColor.isValid()) {
        return mOverlayColor;
    }

    mOverlayColor = gpu::Program(
        mVertexShaderOverlayGeneric,
        gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"COLOR"})
    );

    return mOverlayColor;
}

gpu::Program& ProgramCache::overlay()
{
    if (mOverlay.isValid()) {
        return mOverlay;
    }

    mOverlay = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(GL_FRAGMENT_SHADER, OVERLAY_FRAG)
    );

    return mOverlay;
}

} // namespace render
