/* LightManager.hpp -- Direct light management class for the scene system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_LIGHT_MANAGER_HPP
#define HP_SCENE_LIGHT_MANAGER_HPP

#include <Hyperion/HP_Render.h>

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Detail/GPU/Texture.hpp"
#include "../../Detail/GPU/Buffer.hpp"

#include "../Core/ProgramCache.hpp"
#include "../Core/AssetCache.hpp"

#include "./RenderableBuffer.hpp"
#include "./MaterialBuffer.hpp"
#include "./ViewFrustum.hpp"
#include "./Environment.hpp"
#include "./BoneBuffer.hpp"
#include "../HP_Light.hpp"
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
    LightManager(render::ProgramCache& programs, render::AssetCache& assets, const HP_AppDesc& desc);

    /** Light life-cycle management */
    HP_Light* create(HP_LightType type);
    void destroy(HP_Light* light);

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
    HP_IVec2 clusterSize() const;
    HP_IVec3 clusterCount() const;
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
    class FrameShadowUniform {
    public:
        FrameShadowUniform();
    public:
        /** Upload all data into a single buffer; used for directional/spot lights */
        void upload(const HP_Mat4& lightViewProj, const HP_Vec3& lightPosition, float shadowLambda, float farPlane);
        /** Upload constant data to all rotating buffers; used to initialize omni-light passes */
        void upload(const HP_Vec3& lightPosition, float shadowLambda, float farPlane);
        /** Update only the matrix in the current buffer; used for updating omni-light passes */
        void upload(const HP_Mat4& lightViewProj);
        /** Getter */
        gpu::Buffer& buffer();
    private:
        struct Uniform {
            alignas(16) HP_Mat4 lightViewProj;
            alignas(16) HP_Vec3 lightPosition;
            alignas(4) float shadowLambda;
            alignas(4) float farPlane;
        };
        int mCurrentBuffer{};
        std::array<gpu::Buffer, 3> mBuffers{};
    };

private:
    /** Object Pools */
    util::ObjectPool<HP_Light, 32> mLights{};

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
    FrameShadowUniform mFrameShadowUniform;

    /** Additionnal Data */
    int mShadowResolution{};

    HP_IVec3 mClusterCount{};                   ///< Number of clusters X/Y/Z
    HP_IVec2 mClusterSize{};                    ///< Size of a cluster X/Y

    float mClusterSliceScale{};
    float mClusterSliceBias{};

    int mActiveLightCount{0};
    int mActiveShadow2DCount{0};
    int mActiveShadowCubeCount{0};

    bool mLightDirty{false};                    //< Indicates if the state of active lights has changed, this tells us to re-upload them
    bool mShadowDirty{false};                   //< Indicates if the state of active shadows has changed, this tells us to re-upload them
};

/* === Public Implementation === */

inline HP_Light* LightManager::create(HP_LightType type)
{
    return mLights.create(*this, type);
}

inline void LightManager::destroy(HP_Light* light)
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

inline HP_IVec2 LightManager::clusterSize() const
{
    return mClusterSize;
}

inline HP_IVec3 LightManager::clusterCount() const
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
        HP_INTERNAL_LOG(V, "RENDER: Shadows have been marked dirty to the LightManager");
        mShadowDirty = true;
    }
}

inline void LightManager::markLightDirty()
{
    if (!mLightDirty) {
        HP_INTERNAL_LOG(V, "RENDER: Lights have been marked dirty to the LightManager");
        mLightDirty = true;
    }
}

/* === FrameShadow - Public Implementation === */

inline LightManager::FrameShadowUniform::FrameShadowUniform()
{
    for (gpu::Buffer& buffer : mBuffers) {
        buffer = gpu::Buffer(GL_UNIFORM_BUFFER, sizeof(Uniform), nullptr, GL_DYNAMIC_DRAW);
    }
}

inline void LightManager::FrameShadowUniform::upload(const HP_Mat4& lightViewProj, const HP_Vec3& lightPosition, float shadowLambda, float farPlane)
{
    mCurrentBuffer = (mCurrentBuffer + 1) % mBuffers.size();

    mBuffers[mCurrentBuffer].uploadObject(Uniform {
        .lightViewProj = lightViewProj,
        .lightPosition = lightPosition,
        .shadowLambda = shadowLambda,
        .farPlane = farPlane
    });
}

inline void LightManager::FrameShadowUniform::upload(const HP_Vec3& lightPosition, float shadowLambda, float farPlane)
{
    for (gpu::Buffer& buffer : mBuffers) {
        Uniform data { .lightViewProj = {}, .lightPosition = lightPosition, .shadowLambda = shadowLambda, .farPlane = farPlane };
        buffer.upload(offsetof(Uniform, lightViewProj), sizeof(Uniform) - offsetof(Uniform, lightViewProj), &data.lightPosition);
    }
}

inline void LightManager::FrameShadowUniform::upload(const HP_Mat4& lightViewProj)
{
    mCurrentBuffer = (mCurrentBuffer + 1) % mBuffers.size();
    mBuffers[mCurrentBuffer].upload(0, sizeof(HP_Mat4), &lightViewProj);
}

inline gpu::Buffer& LightManager::FrameShadowUniform::buffer()
{
    return mBuffers[mCurrentBuffer];
}

} // namespace scene

#endif // HP_SCENE_LIGHT_MANAGER_HPP
