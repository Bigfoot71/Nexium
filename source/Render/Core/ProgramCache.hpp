/* ProgramCache.hpp -- Manage internal GPU program storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_RENDER_PROGRAM_CACHE_HPP
#define HP_RENDER_PROGRAM_CACHE_HPP

#include <Hyperion/HP_Render.h>

#include "../../Detail/GPU/Program.hpp"
#include "../../Detail/GPU/Shader.hpp"
#include "../HP_MaterialShader.hpp"

namespace render {

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
    gpu::Program& prepass();
    gpu::Program& forward();
    gpu::Program& skybox();
    gpu::Program& shadow();

    /** Scene post process programs */
    gpu::Program& bloomPost(HP_Bloom mode);
    gpu::Program& output(HP_Tonemap tonemap);
    gpu::Program& bilateralBlur();
    gpu::Program& downsampling();
    gpu::Program& upsampling();
    gpu::Program& ssaoPass();
    gpu::Program& ssaoPost();

    /** Overlay programs */
    gpu::Program& overlayFontBitmap();
    gpu::Program& overlayFontSDF();
    gpu::Program& overlayTexture();
    gpu::Program& overlayColor();
    gpu::Program& overlay();

private:
    /** Cubemap generation */
    gpu::Program mCubemapFromEquirectangular;
    gpu::Program mCubemapIrradiance;
    gpu::Program mCubemapPrefilter;
    gpu::Program mCubemapSkybox;

    /** Scene programs */
    HP_MaterialShader mMaterialShader{};
    gpu::Program mLightCulling{};
    gpu::Program mSkybox{};

    /** Scene post process programs */
    std::array<gpu::Program, HP_BLOOM_COUNT> mBloomPost{};
    std::array<gpu::Program, HP_TONEMAP_COUNT> mOutput{};
    gpu::Program mBilateralBlur{};
    gpu::Program mDownsampling{};
    gpu::Program mUpsampling{};
    gpu::Program mSsaoPass{};
    gpu::Program mSsaoPost{};

    /** Overlay programs */
    gpu::Program mOverlayFontBitmap{};
    gpu::Program mOverlayFontSDF{};
    gpu::Program mOverlayTexture{};
    gpu::Program mOverlayColor{};
    gpu::Program mOverlay{};

private:
    gpu::Shader mVertexShaderOverlayGeneric;
    gpu::Shader mVertexShaderScreen;
    gpu::Shader mVertexShaderCube;
};

} // namespace render

#endif // HP_RENDER_PROGRAM_CACHE_HPP
