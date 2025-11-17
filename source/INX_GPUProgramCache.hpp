/* INX_GPUProgramCache.hpp -- Manage internal GPU program storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_PROGRAM_CACHE_HPP
#define NX_RENDER_PROGRAM_CACHE_HPP

#include <NX/NX_Environment.h>

#include "./Detail/GPU/Program.hpp"
#include "./Detail/GPU/Shader.hpp"

#include <array>

// ============================================================================
// ENUM PROGRAMS
// ============================================================================

enum INX_ProgramID : uint16_t {
    /** Cubemap generation */
    INX_PROG_CUBEMAP_EQUIRECT = 0,
    INX_PROG_CUBEMAP_IRRADIANCE,
    INX_PROG_CUBEMAP_PREFILTER,
    INX_PROG_CUBEMAP_SKYBOX,
    /** Scene */
    INX_PROG_LIGHT_CULLING,
    INX_PROG_SKYBOX,
    /** Bloom generation */
    INX_PROG_BLOOM_DOWNSAMPLE,
    INX_PROG_BLOOM_UPSAMPLE,
    /** Bloom composite */
    INX_PROG_BLOOM_COMPOSITE_MIX,
    INX_PROG_BLOOM_COMPOSITE_ADDITIVE,
    INX_PROG_BLOOM_COMPOSITE_SCREEN,
    /** Screen space effects */
    INX_PROG_SSAO_PASS,
    INX_PROG_SSGI_PASS,
    /** Screen space post process */
    INX_PROG_EDGE_AWARE_ATROUS,
    INX_PROG_EDGE_AWARE_UPSAMPLE,
    INX_PROG_EDGE_AWARE_BLUR,
    /** Output */
    INX_PROG_OUTPUT_LINEAR,
    INX_PROG_OUTPUT_REINHARD,
    INX_PROG_OUTPUT_FILMIC,
    INX_PROG_OUTPUT_ACES,
    INX_PROG_OUTPUT_AGX,
    /** Overlay */
    INX_PROG_OVERLAY,
    /** Generic */
    INX_PROG_SCREEN_QUAD,
    /** Sentinel */
    INX_PROG_COUNT
};

// ============================================================================
// GPU PROGRAM CACHE
// ============================================================================

class INX_GPUProgramCache {
public:
    /** Cubemap generation */
    gpu::Program& GetCubemapFromEquirectangular();
    gpu::Program& GetCubemapIrradiance();
    gpu::Program& GetCubemapPrefilter();
    gpu::Program& GetCubemapSkybox();

    /** Scene programs */
    gpu::Program& GetLightCulling();
    gpu::Program& GetSkybox();

    /** Bloom programs */
    gpu::Program& GetBloomComposite(NX_Bloom mode);
    gpu::Program& GetBloomDownsample();
    gpu::Program& GetBloomUpsample();

    /** Screen space effects */
    gpu::Program& GetSsaoPass();

    /** Screen space post process */
    gpu::Program& GetEdgeAwareBlur();

    /** Scene output program */
    gpu::Program& GetOutput(NX_Tonemap tonemap);

    /** Overlay */
    gpu::Program& GetOverlay();

    /** Generic programs */
    gpu::Program& GetScreenQuad();

    /** Unloading */
    void UnloadAll();

private:
    gpu::Shader& GetVertexShaderScreen();
    gpu::Shader& GetVertexShaderCube();

private:
    std::array<gpu::Program, INX_PROG_COUNT> mPrograms{};
    gpu::Shader mVertexShaderScreen{};
    gpu::Shader mVertexShaderCube{};

};

extern INX_GPUProgramCache INX_Programs;

#endif // NX_RENDER_PROGRAM_CACHE_HPP
