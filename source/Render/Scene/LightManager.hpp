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
#include <NX/NX_Core.h>

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Detail/Util/ObjectRing.hpp"
#include "../../Detail/GPU/Texture.hpp"
#include "../../Detail/GPU/Buffer.hpp"

#include "../Core/ProgramCache.hpp"
#include "../Core/AssetCache.hpp"

#include "./RenderableBuffer.hpp"
#include "./MaterialBuffer.hpp"
#include "./ViewFrustum.hpp"
#include "./Environment.hpp"
#include "./BoneBuffer.hpp"
#include "../NX_Light.hpp"
#include "./DrawCall.hpp"

namespace scene {

/* === Declaration === */

class LightManager {
public:
    struct ProcessParams {
        const ViewFrustum& viewFrustum;
        const Environment& environment;
        RenderableBuffer& renderableBuffer;
        MaterialBuffer& materialBuffer;
        const BoneBuffer& boneBuffer;
        const BucketDrawCalls& drawCalls;
        const ArrayDrawData& drawData;
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
    const gpu::Texture& shadowCube() const;
    const gpu::Texture& shadow2D() const;

    /** Info getters */
    int activeCount() const;
    NX_IVec2 clusterSize() const;
    NX_IVec3 clusterCount() const;
    int maxLightsPerCluster() const;
    float clusterSliceScale() const;
    float clusterSliceBias() const;
    int shadowResolution() const;

    /** State management */
    void markShadowDirty();
    void markLightDirty();

private:
    /** Process functions */
    void updateState(const ProcessParams& params);
    void uploadActive(const ProcessParams& params);
    void computeClusters(const ProcessParams& params);
    void renderShadowMaps(const ProcessParams& params);

private:
    static constexpr float SlicesPerDepthOctave = 3.0f;     ///< Number of depth slices per depth octave
    static constexpr int MaxLightsPerCluster = 32;          ///< Maximum number of lights in a single cluster

private:
    struct FrameShadowUniform {
        alignas(16) NX_Mat4 lightViewProj;
        alignas(16) NX_Vec3 lightPosition;
        alignas(4) float shadowLambda;
        alignas(4) float farPlane;
        alignas(4) float elapsedTime;
    };

private:
    /** Object Pools */
    util::ObjectPool<NX_Light, 32> mLights{};

    /** Shared assets */
    render::ProgramCache& mPrograms;
    render::AssetCache& mAssets;

    /** Shadow Framebuffers and Targets */
    gpu::Framebuffer mFramebufferShadowCube{};
    gpu::Framebuffer mFramebufferShadow2D{};
    gpu::Texture mShadowMapCubeArray{};         ///< Contains world distances in cubemaps
    gpu::Texture mShadowMap2DArray{};           ///< Contains world distances in 2D textures
    gpu::Texture mShadowDepth{};                ///< Common depth buffer for depth testing (TODO: Make it a renderbuffer)

    /** Storage Buffers */
    gpu::Buffer mStorageLights{};               ///< Storage containing the lights
    gpu::Buffer mStorageShadow{};               ///< Storage containing the shadow map layer indices for each light
    gpu::Buffer mStorageClusters{};             ///< Storage containing the tiles (number of lights per tile)
    gpu::Buffer mStorageIndex{};                ///< Storage containing the light indices for each tile
    gpu::Buffer mStorageClusterAABB{};          ///< Storage containing the cluster AABBs (computed during GPU light culling, could be useful later)

    /** Uniform Buffers */
    util::ObjectRing<gpu::Buffer, 3> mFrameShadowUniform;

    /** Additionnal Data */
    int mShadowResolution{};

    NX_IVec3 mClusterCount{};                   ///< Number of clusters X/Y/Z
    NX_IVec2 mClusterSize{};                    ///< Size of a cluster X/Y

    float mClusterSliceScale{};
    float mClusterSliceBias{};

    int mActiveLightCount{0};
    int mActiveShadow2DCount{0};
    int mActiveShadowCubeCount{0};

    bool mLightDirty{false};                    //< Indicates if the state of active lights has changed, this tells us to re-upload them
    bool mShadowDirty{false};                   //< Indicates if the state of active shadows has changed, this tells us to re-upload them
};

/* === Public Implementation === */

inline NX_Light* LightManager::create(NX_LightType type)
{
    return mLights.create(*this, type);
}

inline void LightManager::destroy(NX_Light* light)
{
    if (light != nullptr) {
        if (light->isActive()) {
            mShadowDirty = true;
            mLightDirty = true;
        }
        mLights.destroy(light);
    }
}

inline void LightManager::process(const ProcessParams& params)
{
    updateState(params);
    uploadActive(params);
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

inline const gpu::Texture& LightManager::shadowCube() const
{
    return mShadowMapCubeArray;
}

inline const gpu::Texture& LightManager::shadow2D() const
{
    return mShadowMap2DArray;
}

inline int LightManager::activeCount() const
{
    return mActiveLightCount;
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

inline void LightManager::markShadowDirty()
{
    if (!mShadowDirty) {
        NX_INTERNAL_LOG(V, "RENDER: Shadows have been marked dirty to the LightManager");
        mShadowDirty = true;
    }
}

inline void LightManager::markLightDirty()
{
    if (!mLightDirty) {
        NX_INTERNAL_LOG(V, "RENDER: Lights have been marked dirty to the LightManager");
        mLightDirty = true;
    }
}

} // namespace scene

#endif // NX_SCENE_LIGHT_MANAGER_HPP
