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
#include "../NX_MaterialShader.hpp"

namespace render {

/* === Declaration === */

class ProgramCache {
public:
    ProgramCache();

    /** Material shaders */
    NX_MaterialShader* createMaterialShader(const char* vert, const char* frag);
    void destroyMaterialShader(NX_MaterialShader* shader);

    /** Should be called at the end of 'NX_End3D()' */
    void clearDynamicMaterialBuffers();

    /** Cubemap generation */
    gpu::Program& cubemapFromEquirectangular();
    gpu::Program& cubemapIrradiance();
    gpu::Program& cubemapPrefilter();
    gpu::Program& cubemapSkybox();

    /** Scene programs */
    NX_MaterialShader& materialShader(NX_MaterialShader* shader);
    gpu::Program& lightCulling();
    gpu::Program& skybox();

    /** Scene post process programs */
    gpu::Program& bloomPost(NX_Bloom mode);
    gpu::Program& output(NX_Tonemap tonemap);
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
    /** Shader pools */
    util::ObjectPool<NX_MaterialShader, 32> mMaterialShaders;

    /** Cubemap generation */
    gpu::Program mCubemapFromEquirectangular;
    gpu::Program mCubemapIrradiance;
    gpu::Program mCubemapPrefilter;
    gpu::Program mCubemapSkybox;

    /** Scene programs */
    NX_MaterialShader mMaterialShader{};
    gpu::Program mLightCulling{};
    gpu::Program mSkybox{};

    /** Scene post process programs */
    std::array<gpu::Program, NX_BLOOM_COUNT> mBloomPost{};
    std::array<gpu::Program, NX_TONEMAP_COUNT> mOutput{};
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

/* === Public Implementation === */

inline NX_MaterialShader* ProgramCache::createMaterialShader(const char* vert, const char* frag)
{
    return mMaterialShaders.create(vert, frag);
}

inline void ProgramCache::destroyMaterialShader(NX_MaterialShader* shader)
{
    mMaterialShaders.destroy(shader);
}

inline void ProgramCache::clearDynamicMaterialBuffers()
{
    for (NX_MaterialShader& shader : mMaterialShaders) {
        shader.clearDynamicBuffer();
    }
}

inline NX_MaterialShader& ProgramCache::materialShader(NX_MaterialShader* shader)
{
    return shader ? *shader : mMaterialShader;
}

} // namespace render

#endif // NX_RENDER_PROGRAM_CACHE_HPP
