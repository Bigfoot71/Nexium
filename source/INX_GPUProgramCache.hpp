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

#include "./Detail/Util/Memory.hpp"
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
    /** Bloom post */
    INX_PROG_BLOOM_POST_MIX,
    INX_PROG_BLOOM_POST_ADDITIVE,
    INX_PROG_BLOOM_POST_SCREEN,
    /** Output */
    INX_PROG_OUTPUT_LINEAR,
    INX_PROG_OUTPUT_REINHARD,
    INX_PROG_OUTPUT_FILMIC,
    INX_PROG_OUTPUT_ACES,
    INX_PROG_OUTPUT_AGX,
    /** SSAO */
    INX_PROG_SSAO_BILATERAL_BLUR,
    INX_PROG_DOWNSAMPLING,
    INX_PROG_UPSAMPLING,
    INX_PROG_SSAO_PASS,
    INX_PROG_SSAO_POST,
    /** Overlay/screen */
    INX_PROG_OVERLAY,
    INX_PROG_SCREEN_QUAD,
    /** Sentinel */
    INX_PROG_COUNT
};

// ============================================================================
// GPU PROGRAM CACHE
// ============================================================================

extern class INX_GPUProgramCache {
public:
    /** Cubemap generation */
    gpu::Program& GetCubemapFromEquirectangular();
    gpu::Program& GetCubemapIrradiance();
    gpu::Program& GetCubemapPrefilter();
    gpu::Program& GetCubemapSkybox();

    /** Scene programs */
    gpu::Program& GetLightCulling();
    gpu::Program& GetSkybox();

    /** Scene post process programs */
    gpu::Program& GetBloomPost(NX_Bloom mode);
    gpu::Program& GetOutput(NX_Tonemap tonemap);
    gpu::Program& GetSsaoBilateralBlur();
    gpu::Program& GetDownsampling();
    gpu::Program& GetUpsampling();
    gpu::Program& GetSsaoPass();
    gpu::Program& GetSsaoPost();

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

} INX_Programs;

#endif // NX_RENDER_PROGRAM_CACHE_HPP
