/* ProgramCache.cpp -- Manage internal GPU program storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./ProgramCache.hpp"

#include "../../Assets/ShaderDecoder.hpp"

#include <shaders/screen.vert.h>
#include <shaders/cube.vert.h>

#include <shaders/cubemap_from_equirectangular.frag.h>
#include <shaders/cubemap_irradiance.frag.h>
#include <shaders/cubemap_prefilter.frag.h>
#include <shaders/cubemap_skybox.frag.h>

#include <shaders/light_culling.comp.h>
#include <shaders/skybox.vert.h>
#include <shaders/skybox.frag.h>

#include <shaders/ssao_bilateral_blur.frag.h>
#include <shaders/downsampling.frag.h>
#include <shaders/screen_quad.frag.h>
#include <shaders/upsampling.frag.h>
#include <shaders/bloom_post.frag.h>
#include <shaders/ssao_pass.frag.h>
#include <shaders/ssao_post.frag.h>
#include <shaders/output.frag.h>

#include <shaders/overlay.frag.h>

namespace render {

/* === Public Implementation === */

ProgramCache::ProgramCache()
    : mVertexShaderScreen(GL_VERTEX_SHADER, assets::ShaderDecoder(SCREEN_VERT, SCREEN_VERT_SIZE))
    , mVertexShaderCube(GL_VERTEX_SHADER, assets::ShaderDecoder(CUBE_VERT, CUBE_VERT_SIZE))
{ }

gpu::Program& ProgramCache::cubemapFromEquirectangular()
{
    if (mCubemapFromEquirectangular.isValid()) {
        return mCubemapFromEquirectangular;
    }

    mCubemapFromEquirectangular = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                CUBEMAP_FROM_EQUIRECTANGULAR_FRAG,
                CUBEMAP_FROM_EQUIRECTANGULAR_FRAG_SIZE
            )
        )
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
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                CUBEMAP_IRRADIANCE_FRAG,
                CUBEMAP_IRRADIANCE_FRAG_SIZE
            )
        )
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
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                CUBEMAP_PREFILTER_FRAG,
                CUBEMAP_PREFILTER_FRAG_SIZE
            )
        )
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
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                CUBEMAP_SKYBOX_FRAG,
                CUBEMAP_SKYBOX_FRAG_SIZE
            )
        )
    );

    return mCubemapSkybox;
}

gpu::Program& ProgramCache::lightCulling()
{
    if (mLightCulling.isValid()) {
        return mLightCulling;
    }

    mLightCulling = gpu::Program(
        gpu::Shader(
            GL_COMPUTE_SHADER,
            assets::ShaderDecoder(
                LIGHT_CULLING_COMP,
                LIGHT_CULLING_COMP_SIZE
            )
        )
    );

    return mLightCulling;
}

gpu::Program& ProgramCache::skybox()
{
    if (mSkybox.isValid()) {
        return mSkybox;
    }

    mSkybox = gpu::Program(
        gpu::Shader(
            GL_VERTEX_SHADER,
            assets::ShaderDecoder(
                SKYBOX_VERT,
                SKYBOX_VERT_SIZE
            )
        ),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                SKYBOX_FRAG,
                SKYBOX_FRAG_SIZE
            )
        )
    );

    return mSkybox;
}

gpu::Program& ProgramCache::output(NX_Tonemap tonemap)
{
    if (mOutput[tonemap].isValid()) {
        return mOutput[tonemap];
    }

    const char* tonemapper = "TONEMAPPER TONEMAP_LINEAR";

    switch (tonemap) {
    case NX_TONEMAP_LINEAR:
        break;
    case NX_TONEMAP_REINHARD:
        tonemapper = "TONEMAPPER TONEMAP_REINHARD";
        break;
    case NX_TONEMAP_FILMIC:
        tonemapper = "TONEMAPPER TONEMAP_FILMIC";
        break;
    case NX_TONEMAP_ACES:
        tonemapper = "TONEMAPPER TONEMAP_ACES";
        break;
    case NX_TONEMAP_AGX:
        tonemapper = "TONEMAPPER TONEMAP_AGX";
        break;
    default:
        NX_LOG(W, "RENDER: Unknown tonemap mode (%i); Linear will be used", tonemap);
        break;
    }

    gpu::Shader frag(
        GL_FRAGMENT_SHADER,
        assets::ShaderDecoder(
            OUTPUT_FRAG,
            OUTPUT_FRAG_SIZE
        ),
        {tonemapper}
    );

    mOutput[tonemap] = gpu::Program(mVertexShaderScreen, frag);

    return mOutput[tonemap];
}

gpu::Program& ProgramCache::ssaoBilateralBlur()
{
    if (mSsaoBilateralBlur.isValid()) {
        return mSsaoBilateralBlur;
    }

    mSsaoBilateralBlur = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                SSAO_BILATERAL_BLUR_FRAG,
                SSAO_BILATERAL_BLUR_FRAG_SIZE
            )
        )
    );

    return mSsaoBilateralBlur;
}

gpu::Program& ProgramCache::downsampling()
{
    if (mDownsampling.isValid()) {
        return mDownsampling;
    }

    mDownsampling = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                DOWNSAMPLING_FRAG,
                DOWNSAMPLING_FRAG_SIZE
            )
        )
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
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                UPSAMPLING_FRAG,
                UPSAMPLING_FRAG_SIZE
            )
        )
    );

    return mUpsampling;
}

gpu::Program& ProgramCache::bloomPost(NX_Bloom mode)
{
    SDL_assert(mode != NX_BLOOM_DISABLED);

    if (mBloomPost[mode].isValid()) {
        return mBloomPost[mode];
    }

    const char* bloomMode = "BLOOM_MIX";

    switch (mode) {
    case NX_BLOOM_MIX:
        break;
    case NX_BLOOM_ADDITIVE:
        bloomMode = "BLOOM_ADDITIVE";
        break;
    case NX_BLOOM_SCREEN:
        bloomMode = "BLOOM_SCREEN";
        break;
    default:
        NX_LOG(W, "RENDER: Unknown bloom mode (%i); Mix will be used", mode);
        break;
    }

    mBloomPost[mode] = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                BLOOM_POST_FRAG,
                BLOOM_POST_FRAG_SIZE
            ),
            {bloomMode}
        )
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
            assets::ShaderDecoder(
                SSAO_PASS_FRAG,
                SSAO_PASS_FRAG_SIZE
            )
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
            assets::ShaderDecoder(
                SSAO_POST_FRAG,
                SSAO_POST_FRAG_SIZE
            )
        )
    );

    return mSsaoPost;
}

gpu::Program& ProgramCache::overlay()
{
    if (mOverlay.isValid()) {
        return mOverlay;
    }

    mOverlay = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                OVERLAY_FRAG,
                OVERLAY_FRAG_SIZE
            )
        )
    );

    return mOverlay;
}

gpu::Program& ProgramCache::screenQuad()
{
    if (mScreenQuad.isValid()) {
        return mScreenQuad;
    }

    mScreenQuad = gpu::Program(
        mVertexShaderScreen,
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            assets::ShaderDecoder(
                SCREEN_QUAD_FRAG,
                SCREEN_QUAD_FRAG_SIZE
            )
        )
    );

    return mScreenQuad;
}

} // namespace render
