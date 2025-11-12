/* NX_Light.hpp -- API definition for Nexium's light module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_LIGHT_HPP
#define NX_LIGHT_HPP

#include <NX/NX_Light.h>

#include <NX/NX_Macros.h>
#include <NX/NX_Math.h>
#include <NX/NX_Log.h>

#include <SDL3/SDL_assert.h>
#include <variant>

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
    alignas(4) float opacity{};
};

struct INX_DirectionalLight {
    NX_Vec3 direction{NX_VEC3_FORWARD};
    NX_Vec3 color{NX_VEC3_ONE};
    float energy{1.0f};
    float specular{0.5f};
    float range{8.0f};                      //< Defines the radius around the camera within which shadows are rendered
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
    float slopeBias{0.005f};
    float bias{0.001f};
    float softness{2.0f};
    float opacity{1.0f};
};

struct INX_ShadowLightState {
    NX_Mat4 viewProj{NX_MAT4_IDENTITY};
    int mapIndex{-1};
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

NX_Mat4 INX_GetDirectionalLightViewProj(NX_Light* light, const NX_Vec3& camPosition);
NX_Mat4 INX_GetSpotLightViewProj(NX_Light* light);
NX_Mat4 INX_GetOmniLightViewProj(NX_Light* light, int face);

void INX_FillGPULight(const NX_Light* light, INX_GPULight* gpu, int shadowIndex);
void INX_FillGPUShadow(const NX_Light* light, INX_GPUShadow* gpu);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static const char* INX_GetLightTypeName(NX_LightType type)
{
    static constexpr const char* names[NX_LIGHT_TYPE_COUNT] = {
        "Directional", "Spot", "Omni"
    };

    return names[type];
}

#endif // NX_LIGHT_HPP
