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
#include "../NX_Shader3D.hpp"
#include "../NX_Shader2D.hpp"

namespace render {

/* === Declaration === */

class ProgramCache {
public:
    ProgramCache();

    /** Material shaders */
    NX_Shader3D* createMaterialShader(const char* vert, const char* frag);
    void destroyMaterialShader(NX_Shader3D* shader);

    /** Shape shaders */
    NX_Shader2D* createShader(const char* vert, const char* frag);
    void destroyShader(NX_Shader2D* shader);

    /** Should be called at the end of 'NX_End3D()' / 'NX_End2D()' */
    void clearDynamicMaterialBuffers();
    void clearDynamicBuffers();

    /** Cubemap generation */
    gpu::Program& cubemapFromEquirectangular();
    gpu::Program& cubemapIrradiance();
    gpu::Program& cubemapPrefilter();
    gpu::Program& cubemapSkybox();

    /** Scene programs */
    NX_Shader3D& materialShader(NX_Shader3D* shader);
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

    /** Overlay programs */
    NX_Shader2D& shader(NX_Shader2D* shader);
    gpu::Program& overlay();

    /** Generic programs */
    gpu::Program& screenQuad();

private:
    /** Shader pools */
    util::ObjectPool<NX_Shader3D, 32> mMaterialShaders;
    util::ObjectPool<NX_Shader2D, 32> mShaders;

    /** Cubemap generation */
    gpu::Program mCubemapFromEquirectangular;
    gpu::Program mCubemapIrradiance;
    gpu::Program mCubemapPrefilter;
    gpu::Program mCubemapSkybox;

    /** Scene programs */
    NX_Shader3D mMaterialShader{};
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
    NX_Shader2D mShader{};
    gpu::Program mOverlay{};

    /** Generic programs */
    gpu::Program mScreenQuad{};

private:
    gpu::Shader mVertexShaderScreen;
    gpu::Shader mVertexShaderCube;
};

/* === Public Implementation === */

inline NX_Shader3D* ProgramCache::createMaterialShader(const char* vert, const char* frag)
{
    return mMaterialShaders.create(vert, frag);
}

inline void ProgramCache::destroyMaterialShader(NX_Shader3D* shader)
{
    mMaterialShaders.destroy(shader);
}

inline NX_Shader2D* ProgramCache::createShader(const char* vert, const char* frag)
{
    return mShaders.create(vert, frag);
}

inline void ProgramCache::destroyShader(NX_Shader2D* shader)
{
    mShaders.destroy(shader);
}

inline void ProgramCache::clearDynamicMaterialBuffers()
{
    for (NX_Shader3D& shader : mMaterialShaders) {
        shader.clearDynamicBuffer();
    }
}

inline void ProgramCache::clearDynamicBuffers()
{
    for (NX_Shader2D& shader : mShaders) {
        shader.clearDynamicBuffer();
    }
}

inline NX_Shader3D& ProgramCache::materialShader(NX_Shader3D* shader)
{
    return shader ? *shader : mMaterialShader;
}

inline NX_Shader2D& ProgramCache::shader(NX_Shader2D* shader)
{
    return shader ? *shader : mShader;
}

} // namespace render

#endif // NX_RENDER_PROGRAM_CACHE_HPP
