/* NX_ReflectionProbe.cpp -- API definition for Nexium's reflection probe module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_ReflectionProbe.hpp"
#include "./NX_Cubemap.hpp"

#include "./Detail/GPU/Pipeline.hpp"
#include "./INX_GPUProgramCache.hpp"
#include "./INX_RenderUtils.hpp"
#include "./INX_GlobalPool.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

NX_ReflectionProbe* NX_CreateReflectionProbe(const NX_Cubemap* cubemap)
{
    NX_ReflectionProbe* probe = INX_Pool.Create<NX_ReflectionProbe>();

    /* --- Create textures --- */

    probe->irradiance.gpu = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = GL_RGB16F,
            .data = nullptr,
            .width = 32,
            .height = 32,
            .depth = 0,
            .mipmap = false
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

    probe->prefilter.gpu = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = GL_RGB16F,
            .data = nullptr,
            .width = 128,
            .height = 128,
            .depth = 0,
            .mipmap = true
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR_MIPMAP_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    /* --- Create framebuffers --- */

    probe->irradiance.framebuffer = gpu::Framebuffer({&probe->irradiance.gpu});
    probe->prefilter.framebuffer = gpu::Framebuffer({&probe->prefilter.gpu});

    /* --- Generate maps if cubemap is provided --- */

    if (cubemap != nullptr) {
        NX_UpdateReflectionProbe(probe, cubemap);
    }

    return probe;
}

NX_ReflectionProbe* NX_LoadReflectionProbe(const char* filePath)
{
    NX_Cubemap* cubemap = NX_LoadCubemap(filePath);
    if (cubemap == nullptr) return nullptr;

    NX_ReflectionProbe* probe = NX_CreateReflectionProbe(cubemap);
    NX_DestroyCubemap(cubemap);

    return probe;
}

void NX_DestroyReflectionProbe(NX_ReflectionProbe* probe)
{
    INX_Pool.Destroy(probe);
}

void NX_UpdateReflectionProbe(NX_ReflectionProbe* probe, const NX_Cubemap* cubemap)
{
    /* --- Setup common pipeline states --- */

    gpu::Pipeline pipeline;

    pipeline.BindTexture(0, cubemap->gpu);

    /* --- Generate irradiance --- */

    pipeline.BindFramebuffer(probe->irradiance.framebuffer);
    pipeline.SetViewport(probe->irradiance.framebuffer);

    pipeline.UseProgram(INX_Programs.GetCubemapIrradiance());

    for (int i = 0; i < 6; i++) {
        probe->irradiance.framebuffer.SetColorAttachmentTarget(0, 0, i);
        pipeline.SetUniformMat4(0, INX_GetCubeView(i) * INX_GetCubeProj());
        pipeline.Draw(GL_TRIANGLES, 36);
    }

    /* --- Generate prefilter --- */

    pipeline.BindFramebuffer(probe->prefilter.framebuffer);
    pipeline.SetViewport(probe->prefilter.framebuffer);

    pipeline.UseProgram(INX_Programs.GetCubemapPrefilter());

    pipeline.SetUniformFloat1(1, cubemap->gpu.GetDimensions().x);
    pipeline.SetUniformInt1(2, cubemap->gpu.GetNumLevels());

    int baseSize = probe->prefilter.gpu.GetWidth();
    for (int mip = 0; mip < probe->prefilter.gpu.GetNumLevels(); mip++) {
        int mipSize = std::max(1, baseSize >> mip);
        pipeline.SetViewport(0, 0, mipSize, mipSize);
        pipeline.SetUniformFloat1(3, static_cast<float>(mip) / (probe->prefilter.gpu.GetNumLevels() - 1));
        for (int i = 0; i < 6; i++) {
            probe->prefilter.framebuffer.SetColorAttachmentTarget(0, 0, i, mip);
            pipeline.SetUniformMat4(0, INX_GetCubeView(i) * INX_GetCubeProj());
            pipeline.Draw(GL_TRIANGLES, 36);
        }
    }
}
