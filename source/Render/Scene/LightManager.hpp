/* LightManager.hpp -- Direct light management class for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_LIGHT_MANAGER_HPP
#define NX_SCENE_LIGHT_MANAGER_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Init.h>

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Detail/Util/ObjectRing.hpp"
#include "../../Detail/GPU/Texture.hpp"
#include "../../Detail/GPU/Buffer.hpp"

#include "../Core/ProgramCache.hpp"
#include "../Core/AssetCache.hpp"

#include "./DrawCallManager.hpp"
#include "./ViewFrustum.hpp"
#include "./Environment.hpp"
#include "../NX_Light.hpp"

namespace scene {

/* === Declaration === */

class LightManager {
public:
    struct ProcessParams {
        const ViewFrustum& viewFrustum;
        const Environment& environment;
        DrawCallManager& drawCalls;
    };

public:
    LightManager(render::ProgramCache& programs, render::AssetCache& assets, const NX_AppDesc& desc);

    /** Light life-cycle management */
    NX_Light* create(NX_LightType type);
    void destroy(NX_Light* light);

    /** Lighting state update */
    void process(const ProcessParams& params);

    /** Buffers and textures getters */
    const gpu::Buffer& lightsBuffer() const;
    const gpu::Buffer& shadowBuffer() const;
    const gpu::Buffer& tilesBuffer() const;
    const gpu::Buffer& indexBuffer() const;
    const gpu::Texture& shadowMap(NX_LightType type) const;

    /** Info getters */
    int activeCount() const;
    NX_IVec2 clusterSize() const;
    NX_IVec3 clusterCount() const;
    int maxLightsPerCluster() const;
    float clusterSliceScale() const;
    float clusterSliceBias() const;
    int shadowResolution() const;

private:
    /** Process functions */
    void updateState(const ProcessParams& params);
    void uploadLights(const ProcessParams& params);
    void uploadShadows(const ProcessParams& params);
    void computeClusters(const ProcessParams& params);
    void renderShadowMaps(const ProcessParams& params);

private:
    static constexpr float SlicesPerDepthOctave = 3.0f;     ///< Number of depth slices per depth octave
    static constexpr int MaxLightsPerCluster = 32;          ///< Maximum number of lights in a single cluster

private:
    /** Per-frame data for shadow maps rendering */
    struct FrameShadowUniform {
        alignas(16) NX_Mat4 lightViewProj;
        alignas(16) NX_Vec3 lightPosition;
        alignas(4) float lightRange;
        alignas(4) int32_t lightType;
        alignas(4) float elapsedTime;
    };

    /** Data for active lights */
    struct ActiveLight {
        NX_Light* light;
        int32_t shadowIndex;
    };

    /** Data for active shadows */
    struct ActiveShadow {
        NX_Light* light;
        uint32_t mapIndex;
    };

private:
    /** Object Pools */
    util::ObjectPool<NX_Light, 32> mLights{};

    /** Shared assets */
    render::ProgramCache& mPrograms;
    render::AssetCache& mAssets;

    /** Shadow framebuffers and targets (one per light type) */
    std::array<gpu::Framebuffer, NX_LIGHT_TYPE_COUNT> mFramebufferShadow{};     ///< Contains framebuffers per light type
    std::array<gpu::Texture, NX_LIGHT_TYPE_COUNT> mTargetShadow{};              ///< Contains textures arrays per light type (cubemap for omni-lights)
    gpu::Texture mShadowDepth{};                                                ///< Common depth buffer for depth testing (TODO: Make it a renderbuffer)

    /** Storage buffers */
    gpu::Buffer mStorageLights{};           ///< Active lights (sorted DIR -> SPOT -> OMNI)
    gpu::Buffer mStorageShadow{};           ///< Per-light shadow data
    gpu::Buffer mStorageClusters{};         ///< Per-cluster light counts (numDir, numSpot, numOmni)
    gpu::Buffer mStorageIndex{};            ///< Per-cluster light indices (grouped by type)
    gpu::Buffer mStorageClusterAABB{};      ///< Per-cluster AABBs (computed during culling)

    /** Uniform buffers */
    util::ObjectRing<gpu::Buffer, 3> mFrameShadowUniform; ///< Triple-buffered uniform data for shadow rendering

    /** Per-frame caches */
    util::DynamicArray<ActiveLight> mActiveLights{}; ///< Active lights (pointers + shadow indices), same order as mStorageLights
    util::BucketArray<ActiveShadow, NX_LightType, NX_LIGHT_TYPE_COUNT> mActiveShadows{}; ///< Active shadow-casting lights, bucketed by type, same order as mStorageShadow
    util::BucketArray<uint32_t, NX_LightType, NX_LIGHT_TYPE_COUNT> mShadowNeedingUpdate{}; ///< Indices of lights needing shadow updates, bucketed by type

    /** Additionnal Data */
    int mShadowResolution{};
    NX_IVec3 mClusterCount{};       ///< Number of clusters X/Y/Z
    NX_IVec2 mClusterSize{};        ///< Size of a cluster X/Y
    float mClusterSliceScale{};
    float mClusterSliceBias{};
};

/* === Public Implementation === */

inline NX_Light* LightManager::create(NX_LightType type)
{
    return mLights.create(type);
}

inline void LightManager::destroy(NX_Light* light)
{
    mLights.destroy(light);
}

inline void LightManager::process(const ProcessParams& params)
{
    // NOTE: We clear the caches only at the beginning
    //       because we will need the light counts
    //       after they have been processed

    mActiveLights.clear();
    mActiveShadows.clear();
    mShadowNeedingUpdate.clear();

    updateState(params);
    uploadLights(params);
    uploadShadows(params);
    computeClusters(params);
    renderShadowMaps(params);
}

inline const gpu::Buffer& LightManager::lightsBuffer() const
{
    return mStorageLights;
}

inline const gpu::Buffer& LightManager::shadowBuffer() const
{
    return mStorageShadow;
}

inline const gpu::Buffer& LightManager::tilesBuffer() const
{
    return mStorageClusters;
}

inline const gpu::Buffer& LightManager::indexBuffer() const
{
    return mStorageIndex;
}

inline const gpu::Texture& LightManager::shadowMap(NX_LightType type) const
{
    SDL_assert(type >= 0 && type < mTargetShadow.size());
    return mTargetShadow[type];
}

inline int LightManager::activeCount() const
{
    return mActiveLights.size();
}

inline NX_IVec2 LightManager::clusterSize() const
{
    return mClusterSize;
}

inline NX_IVec3 LightManager::clusterCount() const
{
    return mClusterCount;
}

inline int LightManager::maxLightsPerCluster() const
{
    return MaxLightsPerCluster;
}

inline float LightManager::clusterSliceScale() const
{
    return mClusterSliceScale;
}

inline float LightManager::clusterSliceBias() const
{
    return mClusterSliceBias;
}

inline int LightManager::shadowResolution() const
{
    return mShadowResolution;
}

} // namespace scene

#endif // NX_SCENE_LIGHT_MANAGER_HPP
