/* INX_GPUProgramCache.cpp -- Manage internal GPU program storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./INX_GPUProgramCache.hpp"
#include "./INX_AssetDecoder.hpp"

#include "./Detail/GPU/Program.hpp"
#include "./Detail/GPU/Shader.hpp"

#include <shaders/screen.vert.h>
#include <shaders/cube.vert.h>

#include <shaders/cubemap_from_equirectangular.frag.h>
#include <shaders/cubemap_irradiance.comp.h>
#include <shaders/cubemap_prefilter.comp.h>
#include <shaders/cubemap_skybox.frag.h>

#include <shaders/light_culling.comp.h>
#include <shaders/skybox.vert.h>
#include <shaders/skybox.frag.h>

#include <shaders/edge_aware_blur.frag.h>
#include <shaders/bloom_composite.frag.h>
#include <shaders/bloom_downsample.frag.h>
#include <shaders/bloom_upsample.frag.h>
#include <shaders/screen_quad.frag.h>
#include <shaders/ssao_pass.frag.h>
#include <shaders/output.frag.h>

#include <shaders/overlay.frag.h>

// ============================================================================
// GPU PROGRAM CACHE
// ============================================================================

INX_GPUProgramCache INX_Programs{};

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

gpu::Program& INX_GPUProgramCache::GetCubemapFromEquirectangular()
{
    gpu::Program& program = mPrograms[INX_PROG_CUBEMAP_EQUIRECT];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                CUBEMAP_FROM_EQUIRECTANGULAR_FRAG,
                CUBEMAP_FROM_EQUIRECTANGULAR_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetCubemapIrradiance()
{
    gpu::Program& program = mPrograms[INX_PROG_CUBEMAP_IRRADIANCE];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        gpu::Shader(
            GL_COMPUTE_SHADER,
            INX_ShaderDecoder(
                CUBEMAP_IRRADIANCE_COMP,
                CUBEMAP_IRRADIANCE_COMP_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetCubemapPrefilter()
{
    gpu::Program& program = mPrograms[INX_PROG_CUBEMAP_PREFILTER];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        gpu::Shader(
            GL_COMPUTE_SHADER,
            INX_ShaderDecoder(
                CUBEMAP_PREFILTER_COMP,
                CUBEMAP_PREFILTER_COMP_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetCubemapSkybox()
{
    gpu::Program& program = mPrograms[INX_PROG_CUBEMAP_SKYBOX];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderCube(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                CUBEMAP_SKYBOX_FRAG,
                CUBEMAP_SKYBOX_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetLightCulling()
{
    gpu::Program& program = mPrograms[INX_PROG_LIGHT_CULLING];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        gpu::Shader(
            GL_COMPUTE_SHADER,
            INX_ShaderDecoder(
                LIGHT_CULLING_COMP,
                LIGHT_CULLING_COMP_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetSkybox()
{
    gpu::Program& program = mPrograms[INX_PROG_SKYBOX];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        gpu::Shader(
            GL_VERTEX_SHADER,
            INX_ShaderDecoder(
                SKYBOX_VERT,
                SKYBOX_VERT_SIZE
            )
        ),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                SKYBOX_FRAG,
                SKYBOX_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetBloomComposite(NX_Bloom mode)
{
    SDL_assert(mode != NX_BLOOM_DISABLED);

    INX_ProgramID id{INX_PROG_BLOOM_COMPOSITE_MIX};
    const char* bloomMode = "BLOOM_MIX";

    switch (mode) {
    case NX_BLOOM_MIX:
        break;
    case NX_BLOOM_ADDITIVE:
        id = INX_PROG_BLOOM_COMPOSITE_ADDITIVE;
        bloomMode = "BLOOM_ADDITIVE";
        break;
    case NX_BLOOM_SCREEN:
        id = INX_PROG_BLOOM_COMPOSITE_SCREEN;
        bloomMode = "BLOOM_SCREEN";
        break;
    default:
        break;
    }

    gpu::Program& program = mPrograms[id];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                BLOOM_COMPOSITE_FRAG,
                BLOOM_COMPOSITE_FRAG_SIZE
            ),
            {bloomMode}
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetBloomDownsample()
{
    gpu::Program& program = mPrograms[INX_PROG_BLOOM_DOWNSAMPLE];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                BLOOM_DOWNSAMPLE_FRAG,
                BLOOM_DOWNSAMPLE_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetBloomUpsample()
{
    gpu::Program& program = mPrograms[INX_PROG_BLOOM_UPSAMPLE];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                BLOOM_UPSAMPLE_FRAG,
                BLOOM_UPSAMPLE_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetSsaoPass()
{
    gpu::Program& program = mPrograms[INX_PROG_SSAO_PASS];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                SSAO_PASS_FRAG,
                SSAO_PASS_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetEdgeAwareBlur()
{
    gpu::Program& program = mPrograms[INX_PROG_EDGE_AWARE_BLUR];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                EDGE_AWARE_BLUR_FRAG,
                EDGE_AWARE_BLUR_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetOutput(NX_Tonemap tonemap)
{
    INX_ProgramID id{INX_PROG_OUTPUT_LINEAR};
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

    gpu::Program& program = mPrograms[id];

    if (program.IsValid()) {
        return program;
    }

    gpu::Shader frag(
        GL_FRAGMENT_SHADER,
        INX_ShaderDecoder(
            OUTPUT_FRAG,
            OUTPUT_FRAG_SIZE
        ),
        {tonemapper}
    );

    program = gpu::Program(GetVertexShaderScreen(), frag);

    return program;
}

gpu::Program& INX_GPUProgramCache::GetOverlay()
{
    gpu::Program& program = mPrograms[INX_PROG_OVERLAY];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                OVERLAY_FRAG,
                OVERLAY_FRAG_SIZE
            )
        )
    );

    return program;
}

gpu::Program& INX_GPUProgramCache::GetScreenQuad()
{
    gpu::Program& program = mPrograms[INX_PROG_SCREEN_QUAD];

    if (program.IsValid()) {
        return program;
    }

    program = gpu::Program(
        GetVertexShaderScreen(),
        gpu::Shader(
            GL_FRAGMENT_SHADER,
            INX_ShaderDecoder(
                SCREEN_QUAD_FRAG,
                SCREEN_QUAD_FRAG_SIZE
            )
        )
    );

    return program;
}

void INX_GPUProgramCache::UnloadAll()
{
    for (gpu::Program& program : mPrograms) {
        program = gpu::Program{};
    }
    mVertexShaderScreen = gpu::Shader{};
    mVertexShaderCube = gpu::Shader{};
}

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

gpu::Shader& INX_GPUProgramCache::GetVertexShaderScreen()
{
    if (mVertexShaderScreen.IsValid()) {
        return mVertexShaderScreen;
    }

    mVertexShaderScreen = gpu::Shader(
        GL_VERTEX_SHADER,
        INX_ShaderDecoder(
            SCREEN_VERT,
            SCREEN_VERT_SIZE
        )
    );

    return mVertexShaderScreen;
}

gpu::Shader& INX_GPUProgramCache::GetVertexShaderCube()
{
    if (mVertexShaderCube.IsValid()) {
        return mVertexShaderCube;
    }

    mVertexShaderCube = gpu::Shader(
        GL_VERTEX_SHADER,
        INX_ShaderDecoder(
            CUBE_VERT,
            CUBE_VERT_SIZE
        )
    );

    return mVertexShaderCube;
}
