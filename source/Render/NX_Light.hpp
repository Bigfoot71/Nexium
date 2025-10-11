/* NX_Light.hpp -- Implementation of the API for lights
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_LIGHT_HPP
#define NX_LIGHT_HPP

#include "./Scene/DrawCall.hpp"
#include "./Scene/DrawData.hpp"
#include "./Scene/Frustum.hpp"
#include "./Core/Helper.hpp"

#include <NX/NX_Macros.h>
#include <NX/NX_Render.h>
#include <NX/NX_Math.h>
#include <NX/NX_Core.h>
#include <SDL3/SDL_assert.h>
#include <variant>

/* === Forward Declaration === */

namespace scene {
class LightManager;
}

/* === Declaration === */

class NX_Light {
public:
    /** Light data send to the GPU */
    struct LightGPU {
        alignas(16) NX_Vec3 position{};
        alignas(16) NX_Vec3 direction{};
        alignas(16) NX_Vec3 color{};
        alignas(4) float energy{};
        alignas(4) float specular{};
        alignas(4) float range{};
        alignas(4) float attenuation{};
        alignas(4) float innerCutOff{};
        alignas(4) float outerCutOff{};
        alignas(4) uint32_t layerMask{};        //< Bitmask for camera culling, used in the light culling compute shader
        alignas(4) uint32_t cullMask{};         //< Bitmask used for mesh lighting, used during lighting in the fragment shader
        alignas(4) int32_t shadowIndex{-1};     //< -1 means no shadow
        alignas(4) int32_t type{};
    };

    /** Shadow data send to the GPU */
    struct ShadowGPU {
        alignas(16) NX_Mat4 viewProj{};
        alignas(4) float bleedingBias{};
        alignas(4) float softness{};
        alignas(4) float lambda{};
        alignas(4) uint32_t mapIndex{};
    };

public:
    /** Constructors */
    NX_Light(scene::LightManager& manager, NX_LightType type);

    /** Shadow state management */
    void updateState(const NX_BoundingBox& sceneBounds, uint32_t lightIndex, int32_t shadowIndex, uint32_t shadowMapIndex);
    void forceShadowMapUpdate();
    bool needsShadowMapUpdate();

    /** Public getters */
    NX_LightType type() const;
    bool isActive() const;
    NX_Layer layerMask() const;
    NX_Layer cullMask() const;
    NX_Vec3 position() const;
    NX_Vec3 direction() const;
    NX_Color color() const;
    float energy() const;
    float specular() const;
    float range() const;
    float attenuation() const;
    float innerCutOff() const;
    float outerCutOff() const;
    bool isShadowActive() const;
    NX_Layer shadowCullMask() const;
    float shadowBleedingBias() const;
    float shadowSoftness() const;
    float shadowLambda() const;
    NX_ShadowUpdateMode shadowUpdateMode() const;
    float shadowUpdateInterval() const;

    /** Public setters */
    void setActive(bool active);
    void setLayerMask(NX_Layer layers);
    void setCullMask(NX_Layer layers);
    void setPosition(NX_Vec3 position);
    void setDirection(NX_Vec3 direction);
    void setColor(NX_Color color);
    void setEnergy(float energy);
    void setSpecular(float specular);
    void setRange(float range);
    void setAttenuation(float attenuation);
    void setInnerCutOff(float radians);
    void setOuterCutOff(float radians);
    void setShadowActive(bool active);
    void setShadowCullMask(NX_Layer layers);
    void setShadowBleedingBias(float bias);
    void setShadowSoftness(float softness);
    void setShadowLambda(float lambda);
    void setShadowUpdateMode(NX_ShadowUpdateMode mode);
    void setShadowUpdateInterval(float interval);

    /** Getters for light manager */
    bool isInsideShadowFrustum(const scene::DrawCall& call, const scene::DrawData& data, int face = 0) const;
    void fillShadowGPU(ShadowGPU* shadow) const;
    void fillLightGPU(LightGPU* light) const;
    const NX_Mat4& viewProj(int face = 0);
    uint32_t shadowMapIndex() const;
    int32_t shadowIndex() const;
    uint32_t lightIndex() const;

private:
    void markDirty(bool light, bool shadow, bool viewProj);
    void updateDirectionalViewProj(const NX_BoundingBox& sceneBounds);
    void updateSpotViewProj();
    void updateOmniViewProj();

private:
    struct Directional {
        NX_Vec3 position{NX_VEC3_ZERO};         //< Used for shadow projection
        NX_Vec3 direction{NX_VEC3_FORWARD};
        NX_Vec3 color{NX_VEC3_ONE};
        float energy{1.0f};
        float specular{0.5f};
        float range{0.0f};                      //< Used for shadow projection
    };

    struct Spot {
        NX_Vec3 position{NX_VEC3_ZERO};
        NX_Vec3 direction{NX_VEC3_FORWARD};
        NX_Vec3 color{NX_VEC3_ONE};
        float energy{1.0f};
        float specular{0.5f};
        float range{16.0f};
        float attenuation{1.0f};
        float innerCutOff{0.7071f};             //< ~ 45°
        float outerCutOff{1e-6f};               //< ~ 90°
    };

    struct Omni {
        NX_Vec3 position{NX_VEC3_ZERO};
        NX_Vec3 color{NX_VEC3_ONE};
        float energy{1.0f};
        float specular{0.5f};
        float range{16.0f};
        float attenuation{1.0f};
    };

    using LightData = std::variant<Directional, Spot, Omni>;

    struct ShadowData {
        // NOTE: We store the viewProj matrices and frustums for each face in case of omni-light
        std::array<scene::Frustum, 6> frustum{};
        std::array<NX_Mat4, 6> viewProj{};
        float bleedingBias{};
        float softness{};
        float lambda{};
    };

    struct ShadowState {
        NX_ShadowUpdateMode updateMode{};
        float intervalSec{0.016f};
        float timerSec{0.0f};
        bool needsUpdate{1};
        bool vpDirty{1};
    };

private:
    scene::LightManager& mManager;

private:
    LightData mData;                        //< Union holding data for the specific light type instance
    ShadowData mShadowData{};               //< Shadow data to be uploaded to the GPU
    ShadowState mShadowState{};             //< CPU-side shadow management state
    const NX_LightType mType{};             //< Immutable light type
    uint32_t mLightStorageIndex{0};         //< Index of light data in the SSBO (assigned by manager) if active
    int32_t mShadowStorageIndex{-1};        //< Index of shadow data in the SSBO (-1 if no shadows)
    uint32_t mShadowMapIndex{0};            //< Texture index of the shadow map if shadows are produced
    NX_Layer mLayerMask{NX_LAYER_01};       //< Layers in the scene where the light is active
    NX_Layer mLightCullMask{NX_LAYER_ALL};  //< Layers of meshes affected by this light
    NX_Layer mShadowCullMask{NX_LAYER_ALL}; //< Layers of meshes that produce shadows from this light
    bool mHasShadow{false};                 //< True if the light casts shadows
    bool mActive{false};                    //< True if the light is active
};

/* === Static Asserts === */

static_assert(sizeof(NX_Light::LightGPU) % 16 == 0);    //< std430 compatibility
static_assert(sizeof(NX_Light::ShadowGPU) % 16 == 0);   //< std430 compatibility

/* === Public Implementation === */

inline void NX_Light::updateState(const NX_BoundingBox& sceneBounds, uint32_t lightIndex, int32_t shadowIndex, uint32_t shadowMapIndex)
{
    SDL_assert(mActive);

    mLightStorageIndex = lightIndex;

    if (!mHasShadow) {
        return;
    }

    mShadowStorageIndex = shadowIndex;
    mShadowMapIndex = shadowMapIndex;

    if (mShadowState.vpDirty) {
        switch (mType) {
        case NX_LIGHT_DIR:
            updateDirectionalViewProj(sceneBounds);
            break;
        case NX_LIGHT_SPOT:
            updateSpotViewProj();
            break;
        case NX_LIGHT_OMNI:
            updateOmniViewProj();
            break;
        }
        mShadowState.vpDirty = false;
    }

    if (mShadowState.updateMode == NX_SHADOW_UPDATE_INTERVAL) {
        if (!mShadowState.needsUpdate) {
            mShadowState.timerSec += NX_GetFrameTime();
            if (mShadowState.timerSec >= mShadowState.intervalSec) {
                mShadowState.timerSec -= mShadowState.intervalSec;
                mShadowState.needsUpdate = true;
            }
        }
    }
}

inline void NX_Light::forceShadowMapUpdate()
{
    mShadowState.needsUpdate = true;

    if (mShadowState.updateMode == NX_SHADOW_UPDATE_INTERVAL) {
        mShadowState.timerSec = 0.0f;
    }
}

inline bool NX_Light::needsShadowMapUpdate()
{
    bool result = mShadowState.needsUpdate;

    switch (mShadowState.updateMode) {
    case NX_SHADOW_UPDATE_CONTINUOUS:
        mShadowState.needsUpdate = true;
        break;
    case NX_SHADOW_UPDATE_INTERVAL:
    case NX_SHADOW_UPDATE_MANUAL:
        mShadowState.needsUpdate = false;
        break;
    }

    return result;
}

inline NX_LightType NX_Light::type() const
{
    return mType;
}

inline bool NX_Light::isActive() const
{
    return mActive;
}

inline NX_Layer NX_Light::layerMask() const
{
    return mLayerMask;
}

inline NX_Layer NX_Light::cullMask() const
{
    return mLightCullMask;
}

inline NX_Vec3 NX_Light::position() const
{
    NX_Vec3 result = NX_VEC3_ZERO;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            const Directional& dir = std::get<Directional>(mData);
            result = dir.position; //< Only used for shadow projection
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = spot.position;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const Omni& omni = std::get<Omni>(mData);
            result = omni.position;
        }
        break;
    }

    return result;
}

inline NX_Vec3 NX_Light::direction() const
{
    NX_Vec3 result = NX_VEC3_ZERO;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            const Directional& dir = std::get<Directional>(mData);
            result = dir.direction;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = spot.direction;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot retrieve direction of an omni-directional light (operation ignored)");
        }
        break;
    }

    return result;
}

inline NX_Color NX_Light::color() const
{
    NX_Color result = NX_BLACK;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            const Directional& dir = std::get<Directional>(mData);
            result.r = dir.color.x;
            result.g = dir.color.y;
            result.b = dir.color.z;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result.r = spot.color.x;
            result.g = spot.color.y;
            result.b = spot.color.z;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const Omni& omni = std::get<Omni>(mData);
            result.r = omni.color.x;
            result.g = omni.color.y;
            result.b = omni.color.z;
        }
        break;
    }

    return result;
}

inline float NX_Light::energy() const
{
    float result = 0.0f;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            const Directional& dir = std::get<Directional>(mData);
            result = dir.energy;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = spot.energy;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const Omni& omni = std::get<Omni>(mData);
            result = omni.energy;
        }
        break;
    }

    return result;
}

inline float NX_Light::specular() const
{
    float result = 0.0f;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            const Directional& dir = std::get<Directional>(mData);
            result = dir.specular;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = spot.specular;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const Omni& omni = std::get<Omni>(mData);
            result = omni.specular;
        }
        break;
    }

    return result;
}

inline float NX_Light::range() const
{
    float result = 0.0f;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            const Directional& dir = std::get<Directional>(mData);
            result = dir.range; //< Only used for shadow projection
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = spot.range;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const Omni& omni = std::get<Omni>(mData);
            result = omni.range;
        }
        break;
    }

    return result;
}

inline float NX_Light::attenuation() const
{
    float result = 0.0f;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot retrieve attenuation of a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = spot.attenuation;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const Omni& omni = std::get<Omni>(mData);
            result = omni.attenuation;
        }
        break;
    }

    return result;
}

inline float NX_Light::innerCutOff() const
{
    float result = 0.0f;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot retrieve inner cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = acosf(spot.innerCutOff);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot retrieve inner cutoff to an omni-directional light (operation ignored)");
        }
        break;
    }

    return result;
}

inline float NX_Light::outerCutOff() const
{
    float result = 0.0f;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot retrieve outer cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);
            result = acosf(spot.outerCutOff);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot retrieve outer cutoff to an omni-directional light (operation ignored)");
        }
        break;
    }

    return result;
}

inline bool NX_Light::isShadowActive() const
{
    return mHasShadow;
}

inline NX_Layer NX_Light::shadowCullMask() const
{
    return mShadowCullMask;
}

inline float NX_Light::shadowBleedingBias() const
{
    return mShadowData.bleedingBias;
}

inline float NX_Light::shadowSoftness() const
{
    return mShadowData.softness;
}

inline float NX_Light::shadowLambda() const
{
    return mShadowData.lambda;
}

inline NX_ShadowUpdateMode NX_Light::shadowUpdateMode() const
{
    return mShadowState.updateMode;
}

inline float NX_Light::shadowUpdateInterval() const
{
    return mShadowState.intervalSec;
}

inline void NX_Light::setActive(bool active)
{
    if (mActive != active) {
        markDirty(true, true, false);
        mActive = active;
    }
}

inline void NX_Light::setLayerMask(NX_Layer layers)
{
    if (mLayerMask != layers) {
        markDirty(mActive, false, false);
        mLayerMask = layers;
    }
}

inline void NX_Light::setCullMask(NX_Layer layers)
{
    if (mLightCullMask != layers) {
        markDirty(mActive, false, false);
        mLightCullMask = layers;
    }
}

inline void NX_Light::setPosition(NX_Vec3 position)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign position to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            if (spot.position != position) {
                spot.position = position;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            if (omni.position != position) {
                omni.position = position;
                updated = true;
            }
        }
        break;
    }

    if (updated) {
        // Only spot light position changes affect GPU shadows data; omni VP matrices aren't sent
        markDirty(mActive, mHasShadow && mType == NX_LIGHT_SPOT, true);
    }
}

inline void NX_Light::setDirection(NX_Vec3 direction)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            direction = NX_Vec3Normalize(direction);
            if (dir.direction != direction) {
                dir.direction = direction;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            direction = NX_Vec3Normalize(direction);
            if (spot.direction != direction) {
                spot.direction = direction;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign direction to an omni-directional light (operation ignored)");
        }
        break;
    }

    if (updated) {
        markDirty(mActive, mHasShadow, true);
    }
}

inline void NX_Light::setColor(NX_Color color)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            NX_Vec3 colV3 = NX_VEC3(color.r, color.g, color.b);
            if (dir.color != colV3) {
                dir.color = colV3;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            NX_Vec3 colV3 = NX_VEC3(color.r, color.g, color.b);
            if (spot.color != colV3) {
                spot.color = colV3;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            NX_Vec3 colV3 = NX_VEC3(color.r, color.g, color.b);
            if (omni.color != colV3) {
                omni.color = colV3;
                updated = true;
            }
        }
        break;
    }

    if (updated) {
        markDirty(mActive, false, false);
    }
}

inline void NX_Light::setEnergy(float energy)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            if (dir.energy != energy) {
                dir.energy = energy;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            if (spot.energy != energy) {
                spot.energy = energy;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            if (omni.energy != energy) {
                omni.energy = energy;
                updated = true;
            }
        }
        break;
    }

    if (updated) {
        markDirty(mActive, false, false);
    }
}

inline void NX_Light::setSpecular(float specular)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            if (dir.specular != specular) {
                dir.specular = specular;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            if (spot.specular != specular) {
                spot.specular = specular;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            if (omni.specular != specular) {
                omni.specular = specular;
                updated = true;
            }
        }
        break;
    }

    if (updated) {
        markDirty(mActive, false, false);
    }
}

inline void NX_Light::setRange(float range)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign range to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            if (spot.range != range) {
                spot.range = range;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            if (omni.range != range) {
                omni.range = range;
                updated = true;
            }
        }
        break;
    }

    if (updated) {
        // Only spot light range changes affect GPU shadows data; omni VP matrices aren't sent
        markDirty(mActive, mHasShadow && mType == NX_LIGHT_SPOT, true);
    }
}

inline void NX_Light::setAttenuation(float attenuation)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign attenuation to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            if (spot.attenuation != attenuation) {
                spot.attenuation = attenuation;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            if (omni.attenuation != attenuation) {
                omni.attenuation = attenuation;
                updated = true;
            }
        }
        break;
    }

    if (updated) {
        markDirty(mActive, false, false);
    }
}

inline void NX_Light::setInnerCutOff(float radians)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign inner cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            float cosTh = cosf(radians);
            if (spot.innerCutOff != cosTh) {
                spot.innerCutOff = cosTh;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign inner cutoff to an omni-directional light (operation ignored)");
        }
        break;
    }

    if (updated) {
        markDirty(mActive, false, false);
    }
}

inline void NX_Light::setOuterCutOff(float radians)
{
    bool updated = false;

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign outer cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            float cosTh = cosf(radians);
            if (spot.outerCutOff != cosTh) {
                spot.outerCutOff = cosTh;
                updated = true;
            }
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign outer cutoff to an omni-directional light (operation ignored)");
        }
        break;
    }

    if (updated) {
        markDirty(mActive, mHasShadow, true);
    }
}

inline void NX_Light::setShadowActive(bool active)
{
    if (mHasShadow != active) {
        markDirty(true, true, false);
        mHasShadow = active;
    }
}

inline void NX_Light::setShadowCullMask(NX_Layer layers)
{
    // NOTE: The change will only take effect on the next shadow map rendering,
    //       just like changes in position, direction, or range...

    mShadowCullMask = layers;
}

inline void NX_Light::setShadowBleedingBias(float bias)
{
    if (mShadowData.bleedingBias != bias) {
        markDirty(false, mHasShadow, false);
        mShadowData.bleedingBias = bias;
    }
}

inline void NX_Light::setShadowSoftness(float softness)
{
    if (mShadowData.softness != softness) {
        markDirty(false, mHasShadow, false);
        mShadowData.softness = softness;
    }
}

inline void NX_Light::setShadowLambda(float lambda)
{
    if (mShadowData.lambda != lambda) {
        markDirty(false, mHasShadow, false);
        mShadowData.lambda = lambda;
    }
}

inline void NX_Light::setShadowUpdateMode(NX_ShadowUpdateMode mode)
{
    if (mShadowState.updateMode == mode) {
        return;
    }

    mShadowState.updateMode = mode;

    switch (mode) {
    case NX_SHADOW_UPDATE_CONTINUOUS:
        mShadowState.needsUpdate = true;
        break;
    case NX_SHADOW_UPDATE_INTERVAL:
        mShadowState.needsUpdate = true;
        mShadowState.timerSec = 0.0f;
        break;
    case NX_SHADOW_UPDATE_MANUAL:
        mShadowState.needsUpdate = false;
        break;
    }
}

inline void NX_Light::setShadowUpdateInterval(float interval)
{
    mShadowState.intervalSec = interval;
}

inline bool NX_Light::isInsideShadowFrustum(const scene::DrawCall& call, const scene::DrawData& data, int face) const
{
    // Assert that:
    // - For non-omni lights, only face 0 is valid.
    // - For omni lights, valid faces are 0 through 5.
    SDL_assert((mType != NX_LIGHT_OMNI && face == 0) || (mType == NX_LIGHT_OMNI && face < 5));

    return mShadowData.frustum[face].containsObb(call.aabb(), data.transform());
}

inline void NX_Light::fillShadowGPU(ShadowGPU* shadow) const
{
    SDL_assert(shadow != nullptr);
    SDL_assert(mHasShadow);

    if (mType != NX_LIGHT_OMNI) {
        shadow->viewProj = mShadowData.viewProj[0];
    }

    shadow->bleedingBias = mShadowData.bleedingBias;
    shadow->softness = mShadowData.softness;
    shadow->lambda = mShadowData.lambda;
    shadow->mapIndex = mShadowMapIndex;
}

inline void NX_Light::fillLightGPU(LightGPU* light) const
{
    SDL_assert(light != nullptr);
    SDL_assert(mActive);

    switch (mType) {
    case NX_LIGHT_DIR:
        {
            const Directional& dir = std::get<Directional>(mData);

            light->position = dir.position;
            light->direction = dir.direction;
            light->color = dir.color;
            light->energy = dir.energy;
            light->specular = dir.specular;
            light->range = dir.range;
            light->type = NX_LIGHT_DIR;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const Spot& spot = std::get<Spot>(mData);

            light->position = spot.position;
            light->direction = spot.direction;
            light->color = spot.color;
            light->energy = spot.energy;
            light->specular = spot.specular;
            light->range = spot.range;
            light->attenuation = spot.attenuation;
            light->innerCutOff = spot.innerCutOff;
            light->outerCutOff = spot.outerCutOff;
            light->type = NX_LIGHT_SPOT;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const Omni& omni = std::get<Omni>(mData);

            light->position = omni.position;
            light->color = omni.color;
            light->energy = omni.energy;
            light->specular = omni.specular;
            light->range = omni.range;
            light->attenuation = omni.attenuation;
            light->type = NX_LIGHT_OMNI;
        }
        break;
    }

    light->shadowIndex = mShadowStorageIndex;
    light->cullMask = mLightCullMask;
    light->layerMask = mLayerMask;
}

inline const NX_Mat4& NX_Light::viewProj(int face)
{
    // Assert that:
    // - For non-omni lights, only face 0 is valid.
    // - For omni lights, valid faces are 0 through 5.
    SDL_assert((mType != NX_LIGHT_OMNI && face == 0) || (mType == NX_LIGHT_OMNI && face <= 5));

    return mShadowData.viewProj[face];
}

inline uint32_t NX_Light::shadowMapIndex() const
{
    return mShadowMapIndex;
}

inline int32_t NX_Light::shadowIndex() const
{
    return mShadowStorageIndex;
}

inline uint32_t NX_Light::lightIndex() const
{
    return mLightStorageIndex;
}

#endif // NX_LIGHT_HPP
