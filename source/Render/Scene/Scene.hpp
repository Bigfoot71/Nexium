/* Scene.hpp -- Scene system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_HPP
#define NX_SCENE_HPP

#include <NX/NX_Render.h>

#include "../../Detail/Util/BucketArray.hpp"
#include "../../Detail/GPU/Framebuffer.hpp"
#include "../../Detail/GPU/SwapBuffer.hpp"
#include "../../Detail/GPU/MipBuffer.hpp"
#include "../../Detail/GPU/Texture.hpp"

#include "../Core/ProgramCache.hpp"
#include "../Core/AssetCache.hpp"

#include "../NX_RenderTexture.hpp"
#include "./DrawCallManager.hpp"
#include "./LightManager.hpp"
#include "./Environment.hpp"
#include "./ViewFrustum.hpp"

namespace scene {

/* === Declaration === */

class Scene {
public:
    Scene(render::ProgramCache& programs, render::AssetCache& assets, NX_AppDesc& desc);

    /** Begin/End 3D mode functions */
    void begin(const NX_Camera& camera, const NX_Environment& env, const NX_RenderTexture* target);
    void end();

    /** Getters */
    const DrawCallManager& drawCalls() const;
    DrawCallManager& drawCalls();
    const LightManager& lights() const;
    LightManager& lights();

private:
    /** Scene rendering functions */
    void renderBackground(const gpu::Pipeline& pipeline);
    void renderPrePass(const gpu::Pipeline& pipeline);
    void renderScene(const gpu::Pipeline& pipeline);

    /** Post process functions */
    const gpu::Texture& postSSAO(const gpu::Texture& source);
    const gpu::Texture& postBloom(const gpu::Texture& source);
    void postFinal(const gpu::Texture& source);

private:
    struct TargetInfo {
        const NX_RenderTexture* target{nullptr};
        NX_IVec2 resolution{};
        float aspect{};
    };

    struct FrameUniform {
        alignas(8) NX_IVec2 screenSize;
        alignas(16) NX_IVec3 clusterCount;
        alignas(4) uint32_t maxLightsPerCluster;
        alignas(4) float clusterSliceScale;
        alignas(4) float clusterSliceBias;
        alignas(4) float elapsedTime;
        alignas(4) int32_t hasActiveLights;
        alignas(4) int32_t hasProbe;
    };

private:
    /** Shared assets */
    render::ProgramCache& mPrograms;
    render::AssetCache& mAssets;

    /** Scene data */
    Environment mEnvironment{};
    ViewFrustum mFrustum{};

    /** Managers */
    DrawCallManager mDrawCalls;
    LightManager mLights;

    /** Scene render targets */
    gpu::Texture mTargetSceneColor{};       //< RGBA16F
    gpu::Texture mTargetSceneNormal{};      //< RGB8
    gpu::Texture mTargetSceneDepth{};       //< D24
    gpu::Framebuffer mFramebufferScene{};

    /** Mip chain */
    gpu::MipBuffer mMipChain{};

    /** Swap buffers */
    gpu::SwapBuffer mSwapPostProcess{};     //< Ping-pong buffer used during scene post process
    gpu::SwapBuffer mSwapAuxiliary{};       //< Secondary ping-pong buffer in half resolution

    /** Uniform buffers */
    gpu::Buffer mFrameUniform;

    /** State infos */
    TargetInfo mTargetInfo{};
};

/* === Public Implementation === */

inline const DrawCallManager& Scene::drawCalls() const
{
    return mDrawCalls;
}

inline DrawCallManager& Scene::drawCalls()
{
    return mDrawCalls;
}

inline const LightManager& Scene::lights() const
{
    return mLights;
}

inline LightManager& Scene::lights()
{
    return mLights;
}

} // namespace scene

#endif // NX_SCENE_HPP
