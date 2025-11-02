/* ProgramCache.hpp -- Manage internal GPU program storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_PROGRAM_CACHE_HPP
#define NX_RENDER_PROGRAM_CACHE_HPP

#include <NX/NX_Render.h>

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Detail/GPU/Program.hpp"
#include "../../Detail/GPU/Shader.hpp"

namespace render {

/* === Declaration === */

class ProgramCache {
public:
    ProgramCache();

    /** Cubemap generation */
    gpu::Program& cubemapFromEquirectangular();
    gpu::Program& cubemapIrradiance();
    gpu::Program& cubemapPrefilter();
    gpu::Program& cubemapSkybox();

    /** Scene programs */
    gpu::Program& lightCulling();
    gpu::Program& skybox();

    /** Scene post process programs */
    gpu::Program& bloomPost(NX_Bloom mode);
    gpu::Program& output(NX_Tonemap tonemap);
    gpu::Program& ssaoBilateralBlur();
    gpu::Program& downsampling();
    gpu::Program& upsampling();
    gpu::Program& ssaoPass();
    gpu::Program& ssaoPost();

    /** Generic programs */
    gpu::Program& screenQuad();

private:
    /** Cubemap generation */
    gpu::Program mCubemapFromEquirectangular;
    gpu::Program mCubemapIrradiance;
    gpu::Program mCubemapPrefilter;
    gpu::Program mCubemapSkybox;

    /** Scene programs */
    gpu::Program mLightCulling{};
    gpu::Program mSkybox{};

    /** Scene post process programs */
    std::array<gpu::Program, NX_BLOOM_COUNT> mBloomPost{};
    std::array<gpu::Program, NX_TONEMAP_COUNT> mOutput{};
    gpu::Program mSsaoBilateralBlur{};
    gpu::Program mDownsampling{};
    gpu::Program mUpsampling{};
    gpu::Program mSsaoPass{};
    gpu::Program mSsaoPost{};

    /** Overlay programs */
    gpu::Program mOverlay{};

    /** Generic programs */
    gpu::Program mScreenQuad{};

private:
    gpu::Shader mVertexShaderScreen;
    gpu::Shader mVertexShaderCube;
};

} // namespace render

#endif // NX_RENDER_PROGRAM_CACHE_HPP
