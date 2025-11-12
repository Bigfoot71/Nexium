/* NX_ReflectionProbe.cpp -- API definition for Nexium's reflection probe module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_ReflectionProbe.hpp"
#include "./NX_Render3D.hpp"

#include "./INX_GPUProgramCache.hpp"
#include "./INX_GlobalPool.hpp"

#include "./Detail/GPU/Pipeline.hpp"
#include "./Detail/GPU/Texture.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

NX_ReflectionProbe* NX_CreateReflectionProbe(const NX_Cubemap* cubemap)
{
    NX_ReflectionProbe* probe = INX_Pool.Create<NX_ReflectionProbe>();

    probe->probeIndex = INX_Render3DState_RequestProbe();

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
    INX_Render3DState_ReleaseProbe(probe->probeIndex);
    INX_Pool.Destroy(probe);
}

void NX_UpdateReflectionProbe(NX_ReflectionProbe* probe, const NX_Cubemap* cubemap)
{
    gpu::Pipeline pipeline;
    pipeline.BindTexture(0, cubemap->gpu);

    /* --- Get cubemaps stored in the Render3D state --- */

    const gpu::Texture& irradiance = INX_Render3DState_GetIrradianceArray();
    const gpu::Texture& prefilter = INX_Render3DState_GetPrefilterArray();

    /* --- Generate irradiance --- */

    pipeline.UseProgram(INX_Programs.GetCubemapIrradiance());

    pipeline.BindImageTexture(1, irradiance, 0, -1, GL_WRITE_ONLY);
    pipeline.SetUniformInt1(0, probe->probeIndex);

    int irradianceSize = irradiance.GetWidth();
    int groupsX = NX_DIV_CEIL(irradianceSize, 8);
    int groupsY = NX_DIV_CEIL(irradianceSize, 8);
    int groupsZ = 1;

    pipeline.DispatchCompute(groupsX, groupsY, groupsZ);

    /* --- Generate prefilter --- */

    pipeline.UseProgram(INX_Programs.GetCubemapPrefilter());
    pipeline.SetUniformInt1(0, probe->probeIndex);

    int baseSize = prefilter.GetWidth();
    for (int mip = 0; mip < prefilter.GetNumLevels(); mip++)
    {
        pipeline.BindImageTexture(1, prefilter, mip, -1, GL_WRITE_ONLY);

        float roughness = static_cast<float>(mip) / (prefilter.GetNumLevels() - 1);
        pipeline.SetUniformFloat1(1, roughness);

        int mipSize = std::max(1, baseSize >> mip);
        int groupsX = NX_DIV_CEIL(mipSize, 8);
        int groupsY = NX_DIV_CEIL(mipSize, 8);
        groupsZ = 1;

        pipeline.DispatchCompute(groupsX, groupsY, groupsZ);
    }
}
