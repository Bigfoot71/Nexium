/* NX_ReflectionProbe.cpp -- API definition for Nexium's reflection probe module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_ReflectionProbe.hpp"
#include "./NX_Cubemap.hpp"

#include "./INX_GPUProgramCache.hpp"
#include "./INX_RenderUtils.hpp"
#include "./INX_GlobalPool.hpp"

#include "./Detail/GPU/Pipeline.hpp"
#include "./Detail/GPU/Texture.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

NX_ReflectionProbe* NX_CreateReflectionProbe(const NX_Cubemap* cubemap)
{
    NX_ReflectionProbe* probe = INX_Pool.Create<NX_ReflectionProbe>();

    /* --- Create textures --- */

    probe->irradiance = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = GL_RGBA16F,
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

    probe->prefilter = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = GL_RGBA16F,
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
    gpu::Pipeline pipeline;
    pipeline.BindTexture(0, cubemap->gpu);

    /* --- Generate irradiance --- */

    pipeline.UseProgram(INX_Programs.GetCubemapIrradiance());
    pipeline.BindImageTexture(1, probe->irradiance, 0, -1, GL_WRITE_ONLY);

    int irradianceSize = probe->irradiance.GetWidth();
    int groupsX = NX_DIV_CEIL(irradianceSize, 8);
    int groupsY = NX_DIV_CEIL(irradianceSize, 8);
    int groupsZ = 1;

    pipeline.DispatchCompute(groupsX, groupsY, groupsZ);

    /* --- Generate prefilter --- */

    pipeline.UseProgram(INX_Programs.GetCubemapPrefilter());
    pipeline.SetUniformFloat1(1, cubemap->gpu.GetDimensions().x);
    pipeline.SetUniformInt1(2, cubemap->gpu.GetNumLevels());

    int baseSize = probe->prefilter.GetWidth();
    for (int mip = 0; mip < probe->prefilter.GetNumLevels(); mip++)
    {
        pipeline.BindImageTexture(1, probe->prefilter, mip, -1, GL_WRITE_ONLY);

        float roughness = static_cast<float>(mip) / (probe->prefilter.GetNumLevels() - 1);
        pipeline.SetUniformFloat1(3, roughness);
        pipeline.SetUniformFloat1(4, static_cast<float>(mip));

        int mipSize = std::max(1, baseSize >> mip);
        int groupsX = NX_DIV_CEIL(mipSize, 8);
        int groupsY = NX_DIV_CEIL(mipSize, 8);
        groupsZ = 1;

        pipeline.DispatchCompute(groupsX, groupsY, groupsZ);
    }
}
