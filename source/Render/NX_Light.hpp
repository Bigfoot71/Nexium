/* NX_Light.hpp -- Implementation of the API for lights
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_LIGHT_HPP
#define NX_LIGHT_HPP

#include "./Scene/ViewFrustum.hpp"
#include "./Scene/Frustum.hpp"

#include <NX/NX_Macros.h>
#include <NX/NX_Render.h>
#include <NX/NX_Math.h>
#include <NX/NX_Core.h>

#include <SDL3/SDL_assert.h>
#include <variant>
#include <array>

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
        alignas(4) uint32_t mapIndex{};
        alignas(4) float slopeBias{};
        alignas(4) float bias{};
        alignas(4) float softness{};
    };

public:
    /** Constructors */
    NX_Light(NX_LightType type);

    /** Actions */
    void forceShadowMapUpdate();

    /** Getters */
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
    float shadowSlopeBias() const;
    float shadowBias() const;
    float shadowSoftness() const;
    NX_ShadowUpdateMode shadowUpdateMode() const;
    float shadowUpdateInterval() const;

    /** Setters */
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
    void setShadowSlopeBias(float slopeBias);
    void setShadowBias(float bias);
    void setShadowSoftness(float softness);
    void setShadowUpdateMode(NX_ShadowUpdateMode mode);
    void setShadowUpdateInterval(float interval);

    /** Getters for light manager */
    void updateState(const scene::ViewFrustum& viewFrustum, bool* needsShadowUpdate);
    void fillShadowGPU(ShadowGPU* shadow, int mapIndex) const;
    void fillLightGPU(LightGPU* light, int shadowIndex) const;
    const scene::Frustum& frustum(int face = 0) const;
    const NX_Mat4& viewProj(int face = 0) const;

private:
    void updateDirectionalViewProj(const scene::ViewFrustum& viewFrustum);
    void updateSpotViewProj();
    void updateOmniViewProj();

private:
    struct Directional {
        NX_Vec3 position{NX_VEC3_ZERO};         //< Internally computed, actual position used to build the light's view matrix for shadow projection
        NX_Vec3 direction{NX_VEC3_FORWARD};
        NX_Vec3 color{NX_VEC3_ONE};
        float energy{1.0f};
        float specular{0.5f};
        float shadowRadius{8.0f};               //< Public 'range' parameter, defines the radius around the camera within which shadows are rendered
        float range{0.0f};                      //< Internally computed, corresponds to the shadow projection range (far - near)
    };

    struct Spot {
        NX_Vec3 position{NX_VEC3_ZERO};
        NX_Vec3 direction{NX_VEC3_FORWARD};
        NX_Vec3 color{NX_VEC3_ONE};
        float energy{1.0f};
        float specular{0.5f};
        float range{8.0f};
        float attenuation{1.0f};
        float innerCutOff{0.7071f};             //< ~ 45°
        float outerCutOff{1e-6f};               //< ~ 90°
    };

    struct Omni {
        NX_Vec3 position{NX_VEC3_ZERO};
        NX_Vec3 color{NX_VEC3_ONE};
        float energy{1.0f};
        float specular{0.5f};
        float range{8.0f};
        float attenuation{1.0f};
    };

    using LightData = std::variant<Directional, Spot, Omni>;

    struct ShadowData {
        // NOTE: We store the viewProj matrices and frustums for each face in case of omni-light
        std::array<scene::Frustum, 6> frustum{};
        std::array<NX_Mat4, 6> viewProj{};
        float slopeBias{0.005f};
        float bias{0.001f};
        float softness{2.0f};
    };

    struct ShadowState {
        NX_ShadowUpdateMode updateMode{};
        float intervalSec{0.016f};
        float timerSec{0.0f};
        bool forceUpdate{};
        bool vpDirty{true};
    };

private:
    LightData mData;                        //< Union holding data for the specific light type instance
    ShadowData mShadowData{};               //< Shadow data to be uploaded to the GPU
    ShadowState mShadowState{};             //< CPU-side shadow management state
    const NX_LightType mType{};             //< Immutable light type
    NX_Layer mLayerMask{NX_LAYER_01};       //< Layers in the scene where the light is active
    NX_Layer mLightCullMask{NX_LAYER_ALL};  //< Layers of meshes affected by this light
    NX_Layer mShadowCullMask{NX_LAYER_ALL}; //< Layers of meshes that produce shadows from this light
    bool mHasShadow{false};                 //< True if the light casts shadows
    bool mActive{false};                    //< True if the light is active
};

/* === Public Implementation === */

inline NX_Light::NX_Light(NX_LightType type)
    : mData(Directional())
    , mType(type)
{
    switch (type) {
    case NX_LIGHT_DIR:
        mData = Directional();
        break;
    case NX_LIGHT_SPOT:
        mData = Spot();
        break;
    case NX_LIGHT_OMNI:
        mData = Omni();
        break;
    default:
        NX_INTERNAL_LOG(W, "RENDER: Invalid light type (%i); The light will be invalid");
        break;
    }
}

inline void NX_Light::forceShadowMapUpdate()
{
    mShadowState.forceUpdate = true;

    if (mShadowState.updateMode == NX_SHADOW_UPDATE_INTERVAL) {
        mShadowState.timerSec = 0.0f;
    }
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

inline float NX_Light::shadowSlopeBias() const
{
    return mShadowData.slopeBias;
}

inline float NX_Light::shadowBias() const
{
    return mShadowData.bias;
}

inline float NX_Light::shadowSoftness() const
{
    return mShadowData.softness;
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
    mActive = active;
}

inline void NX_Light::setLayerMask(NX_Layer layers)
{
    mLayerMask = layers;
}

inline void NX_Light::setCullMask(NX_Layer layers)
{
    mLightCullMask = layers;
}

inline void NX_Light::setPosition(NX_Vec3 position)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign position to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            mShadowState.vpDirty = true;
            Spot& spot = std::get<Spot>(mData);
            spot.position = position;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            mShadowState.vpDirty = true;
            Omni& omni = std::get<Omni>(mData);
            omni.position = position;
        }
        break;
    }
}

inline void NX_Light::setDirection(NX_Vec3 direction)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            dir.direction = NX_Vec3Normalize(direction);
        }
        break;
    case NX_LIGHT_SPOT:
        {
            mShadowState.vpDirty = true;
            Spot& spot = std::get<Spot>(mData);
            spot.direction = NX_Vec3Normalize(direction);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign direction to an omni-directional light (operation ignored)");
        }
        break;
    }
}

inline void NX_Light::setColor(NX_Color color)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            dir.color = NX_VEC3(color.r, color.g, color.b);
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            spot.color = NX_VEC3(color.r, color.g, color.b);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            omni.color = NX_VEC3(color.r, color.g, color.b);
        }
        break;
    }
}

inline void NX_Light::setEnergy(float energy)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            dir.energy = energy;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            spot.energy = energy;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            omni.energy = energy;
        }
        break;
    }
}

inline void NX_Light::setSpecular(float specular)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            dir.specular = specular;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            spot.specular = specular;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            omni.specular = specular;
        }
        break;
    }
}

inline void NX_Light::setRange(float range)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            Directional& dir = std::get<Directional>(mData);
            dir.shadowRadius = range;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            mShadowState.vpDirty = true;
            Spot& spot = std::get<Spot>(mData);
            spot.range = range;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            mShadowState.vpDirty = true;
            Omni& omni = std::get<Omni>(mData);
            omni.range = range;
        }
        break;
    }
}

inline void NX_Light::setAttenuation(float attenuation)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign attenuation to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            spot.attenuation = attenuation;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            Omni& omni = std::get<Omni>(mData);
            omni.attenuation = attenuation;
        }
        break;
    }
}

inline void NX_Light::setInnerCutOff(float radians)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign inner cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            Spot& spot = std::get<Spot>(mData);
            spot.innerCutOff = cosf(radians);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign inner cutoff to an omni-directional light (operation ignored)");
        }
        break;
    }
}

inline void NX_Light::setOuterCutOff(float radians)
{
    switch (mType) {
    case NX_LIGHT_DIR:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign outer cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            mShadowState.vpDirty = true;
            Spot& spot = std::get<Spot>(mData);
            spot.outerCutOff = cosf(radians);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_INTERNAL_LOG(W, "RENDER: Cannot assign outer cutoff to an omni-directional light (operation ignored)");
        }
        break;
    }
}

inline void NX_Light::setShadowActive(bool active)
{
    mHasShadow = active;
}

inline void NX_Light::setShadowCullMask(NX_Layer layers)
{
    // NOTE: The change will only take effect on the next shadow map rendering,
    //       just like changes in position, direction, or range...

    mShadowCullMask = layers;
}

inline void NX_Light::setShadowSlopeBias(float slopeBias)
{
    mShadowData.slopeBias = slopeBias;
}

inline void NX_Light::setShadowBias(float bias)
{
    mShadowData.bias = bias;
}

inline void NX_Light::setShadowSoftness(float softness)
{
    mShadowData.softness = softness;
}

inline void NX_Light::setShadowUpdateMode(NX_ShadowUpdateMode mode)
{
    mShadowState.updateMode = mode;
}

inline void NX_Light::setShadowUpdateInterval(float interval)
{
    mShadowState.intervalSec = interval;
}

inline void NX_Light::fillShadowGPU(ShadowGPU* shadow, int mapIndex) const
{
    SDL_assert(shadow != nullptr);
    SDL_assert(mHasShadow);

    if (mType != NX_LIGHT_OMNI) {
        shadow->viewProj = mShadowData.viewProj[0];
    }

    shadow->mapIndex = mapIndex;
    shadow->slopeBias = mShadowData.slopeBias;
    shadow->bias = mShadowData.bias;
    shadow->softness = mShadowData.softness;
}

inline void NX_Light::fillLightGPU(LightGPU* light, int shadowIndex) const
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

    light->shadowIndex = shadowIndex;
    light->cullMask = mLightCullMask;
    light->layerMask = mLayerMask;
}

inline const scene::Frustum& NX_Light::frustum(int face) const
{
    // Assert that:
    // - For non-omni lights, only face 0 is valid.
    // - For omni lights, valid faces are 0 through 5.
    SDL_assert((mType != NX_LIGHT_OMNI && face == 0) || (mType == NX_LIGHT_OMNI && face <= 5));

    return mShadowData.frustum[face];
}

inline const NX_Mat4& NX_Light::viewProj(int face) const
{
    // Assert that:
    // - For non-omni lights, only face 0 is valid.
    // - For omni lights, valid faces are 0 through 5.
    SDL_assert((mType != NX_LIGHT_OMNI && face == 0) || (mType == NX_LIGHT_OMNI && face <= 5));

    return mShadowData.viewProj[face];
}

#endif // NX_LIGHT_HPP
