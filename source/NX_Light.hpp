/* NX_Light.hpp -- API definition for Nexium's light module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_LIGHT_HPP
#define NX_LIGHT_HPP

#include "./Render/Scene/ViewFrustum.hpp"
#include "./Render/Scene/Frustum.hpp"
#include "NX/NX_Light.h"

#include <NX/NX_Macros.h>
#include <NX/NX_Render.h>
#include <NX/NX_Math.h>

#include <SDL3/SDL_assert.h>
#include <variant>
#include <array>

// ============================================================================
// INTERNAL TYPES
// ============================================================================

struct INX_GPULight {
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

struct INX_GPUShadow {
    alignas(16) NX_Mat4 viewProj{};
    alignas(4) uint32_t mapIndex{};
    alignas(4) float slopeBias{};
    alignas(4) float bias{};
    alignas(4) float softness{};
};

struct INX_DirectionalLight {
    NX_Vec3 position{NX_VEC3_ZERO};         //< Internally computed, actual position used to build the light's view matrix for shadow projection
    NX_Vec3 direction{NX_VEC3_FORWARD};
    NX_Vec3 color{NX_VEC3_ONE};
    float energy{1.0f};
    float specular{0.5f};
    float shadowRadius{8.0f};               //< Public 'range' parameter, defines the radius around the camera within which shadows are rendered
    float range{0.0f};                      //< Internally computed, corresponds to the shadow projection range (far - near)
};

struct INX_SpotLight {
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

struct INX_OmniLight {
    NX_Vec3 position{NX_VEC3_ZERO};
    NX_Vec3 color{NX_VEC3_ONE};
    float energy{1.0f};
    float specular{0.5f};
    float range{8.0f};
    float attenuation{1.0f};
};

using INX_LightData = std::variant<
    INX_DirectionalLight,
    INX_SpotLight,
    INX_OmniLight
>;

struct INX_ShadowLightData {
    // NOTE: We store the viewProj matrices and frustums for each face in case of omni-light
    std::array<scene::Frustum, 6> frustum{};
    std::array<NX_Mat4, 6> viewProj{};
    float slopeBias{0.005f};
    float bias{0.001f};
    float softness{2.0f};
};

struct INX_ShadowLightState {
    NX_ShadowUpdateMode updateMode{};
    float intervalSec{0.016f};
    float timerSec{0.0f};
    bool forceUpdate{};
    bool vpDirty{true};
};

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

struct NX_Light {
    /** Light data */
    NX_LightType type{};                //< Light type
    INX_LightData data{};               //< Union holding data for the specific light type instance
    NX_Layer layerMask{NX_LAYER_01};    //< Layers in the scene where the light is active
    NX_Layer cullMask{NX_LAYER_ALL};    //< Layers of meshes affected by this light
    bool active{false};                 //< True if the light is active

    /** shadow data */
    struct {
        INX_ShadowLightData data{};         //< Shadow data to be uploaded to the GPU
        INX_ShadowLightState state{};       //< CPU-side shadow management state
        NX_Layer cullMask{NX_LAYER_ALL};    //< Layers of meshes that produce shadows from this light
        bool active{false};                 //< True if the light casts shadows
    } shadow;

    /** Constructors */
    NX_Light(NX_LightType type);
};

inline NX_Light::NX_Light(NX_LightType type)
    : type(type), data(INX_DirectionalLight())
{
    switch (type) {
    case NX_LIGHT_DIR:
        data = INX_DirectionalLight();
        break;
    case NX_LIGHT_SPOT:
        data = INX_SpotLight();
        break;
    case NX_LIGHT_OMNI:
        data = INX_OmniLight();
        break;
    default:
        NX_LOG(W, "RENDER: Invalid light type (%i); The light will be directional");
        type = NX_LIGHT_DIR;
        break;
    }
}

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

void INX_UpdateLight(NX_Light* light, const scene::ViewFrustum& viewFrustum, bool* needsShadowUpdate);
void INX_FillGPULight(NX_Light* light, INX_GPULight* gpu, int shadowIndex);
void INX_FillGPUShadow(NX_Light* light, INX_GPUShadow* gpu, int mapIndex);

const scene::Frustum& INX_GetLightFrustum(const NX_Light& light, int face);
const NX_Mat4& INX_GetLightViewProj(const NX_Light& light, int face);

#endif // NX_LIGHT_HPP
