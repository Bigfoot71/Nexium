/* NX_Light.cpp -- API definition for Nexium's light module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Light.hpp"

#include <NX/NX_Runtime.h>

#include "./INX_RenderUtils.hpp"
#include "./INX_GlobalPool.hpp"
#include "./NX_Render3D.hpp"
#include <cmath>

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

NX_Mat4 INX_GetDirectionalLightViewProj(NX_Light* light, const NX_Vec3& camPosition)
{
    SDL_assert(light->type == NX_LIGHT_DIR);
    SDL_assert(light->shadow.active);

    const INX_DirectionalLight& dirLight = std::get<INX_DirectionalLight>(light->data);
    const NX_Vec3& lightDir = dirLight.direction;
    const float extent = dirLight.range;

    /* --- Create an orthonormal basis for light --- */

    NX_Vec3 up = (std::abs(NX_Vec3Dot(lightDir, NX_VEC3_UP)) > 0.99f) ? NX_VEC3_BACK : NX_VEC3_UP;
    NX_Vec3 lightRight = NX_Vec3Normalize(NX_Vec3Cross(up, lightDir));
    NX_Vec3 lightUp = NX_Vec3Cross(lightDir, lightRight);

    /* --- Project the camera's position into light space --- */

    float camX = NX_Vec3Dot(camPosition, lightRight);
    float camY = NX_Vec3Dot(camPosition, lightUp);
    float camZ = NX_Vec3Dot(camPosition, lightDir);

    /* --- Snap to the texel grid --- */

    float shadowMapSize = INX_Render3DState_GetShadowMapResolution(NX_LIGHT_DIR);
    float worldUnitsPerTexel = (2.0f * extent) / shadowMapSize;

    float snappedX = std::floor(camX / worldUnitsPerTexel) * worldUnitsPerTexel;
    float snappedY = std::floor(camY / worldUnitsPerTexel) * worldUnitsPerTexel;

    /* --- Reconstruct the snapped world space position --- */

    NX_Vec3 lightPosition = lightRight * snappedX + 
                            lightUp * snappedY + 
                            lightDir * camZ;

    /* --- Construct view and projection --- */

    NX_Mat4 view = NX_Mat4LookTo(lightPosition, lightDir, lightUp);

    NX_Mat4 proj = NX_Mat4Ortho(
        -extent, +extent,
        -extent, +extent,
        -extent, +extent
    );

    /* --- Return the final matrix --- */

    light->shadow.state.viewProj = view * proj;

    return light->shadow.state.viewProj;
}

NX_Mat4 INX_GetSpotLightViewProj(NX_Light* light)
{
    SDL_assert(light->type == NX_LIGHT_SPOT);
    SDL_assert(light->shadow.active);

    const INX_SpotLight& spotLight = std::get<INX_SpotLight>(light->data);

    constexpr float nearPlane = 0.05f;
    NX_Mat4 view = NX_Mat4LookAt(spotLight.position, spotLight.position + spotLight.direction, NX_VEC3_UP);
    NX_Mat4 proj = NX_Mat4Perspective(NX_PI / 2.0f, 1.0f, nearPlane, nearPlane + spotLight.range);

    light->shadow.state.viewProj = view * proj;

    return light->shadow.state.viewProj;
}

NX_Mat4 INX_GetOmniLightViewProj(NX_Light* light, int face)
{
    SDL_assert(light->type == NX_LIGHT_OMNI);
    SDL_assert(light->shadow.active);

    const INX_OmniLight& omniLight = std::get<INX_OmniLight>(light->data);

    constexpr float nearPlane = 0.05f;
    NX_Mat4 view = INX_GetCubeView(face, omniLight.position);
    NX_Mat4 proj = INX_GetCubeProj(nearPlane, nearPlane + omniLight.range);

    light->shadow.state.viewProj = view * proj;

    return light->shadow.state.viewProj;
}

void INX_FillGPULight(const NX_Light* light, INX_GPULight* gpu, int shadowIndex)
{
    SDL_assert(light != nullptr && gpu != nullptr);
    SDL_assert(light->active);

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);

            gpu->direction = dir.direction;
            gpu->color = dir.color;
            gpu->energy = dir.energy;
            gpu->specular = dir.specular;
            gpu->range = dir.range;
            gpu->type = NX_LIGHT_DIR;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);

            gpu->position = spot.position;
            gpu->direction = spot.direction;
            gpu->color = spot.color;
            gpu->energy = spot.energy;
            gpu->specular = spot.specular;
            gpu->range = spot.range;
            gpu->attenuation = spot.attenuation;
            gpu->innerCutOff = spot.innerCutOff;
            gpu->outerCutOff = spot.outerCutOff;
            gpu->type = NX_LIGHT_SPOT;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);

            gpu->position = omni.position;
            gpu->color = omni.color;
            gpu->energy = omni.energy;
            gpu->specular = omni.specular;
            gpu->range = omni.range;
            gpu->attenuation = omni.attenuation;
            gpu->type = NX_LIGHT_OMNI;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    gpu->shadowIndex = shadowIndex;
    gpu->cullMask = light->cullMask;
}

void INX_FillGPUShadow(const NX_Light* light, INX_GPUShadow* gpu)
{
    SDL_assert(light != nullptr && gpu != nullptr);
    SDL_assert(light->shadow.active);
    SDL_assert(light->active);

    if (light->type != NX_LIGHT_OMNI) {
        gpu->viewProj = light->shadow.state.viewProj;
    }

    gpu->mapIndex = light->shadow.state.mapIndex;
    gpu->slopeBias = light->shadow.data.slopeBias;
    gpu->bias = light->shadow.data.bias;
    gpu->softness = light->shadow.data.softness;
    gpu->opacity = light->shadow.data.opacity;
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Light* NX_CreateLight(NX_LightType type)
{
    return INX_Pool.Create<NX_Light>(type);
}

void NX_DestroyLight(NX_Light* light)
{
    INX_Pool.Destroy(light);
}

bool NX_IsLightActive(const NX_Light* light)
{
    return light->active;
}

void NX_SetLightActive(NX_Light* light, bool active)
{
    light->active = active;
}

NX_Layer NX_GetLightLayerMask(const NX_Light* light)
{
    return light->layerMask;
}

void NX_SetLightLayerMask(NX_Light* light, NX_Layer layers)
{
    light->layerMask = layers;
}

NX_Layer NX_GetLightCullMask(const NX_Light* light)
{
    return light->cullMask;
}

void NX_SetLightCullMask(NX_Light* light, NX_Layer layers)
{
    light->cullMask = layers;
}

NX_Vec3 NX_GetLightPosition(const NX_Light* light)
{
    NX_Vec3 result = NX_VEC3_ZERO;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot retrieve position of a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = spot.position;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            result = omni.position;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightPosition(NX_Light* light, NX_Vec3 position)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot assign position to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.position = position;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            omni.position = position;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

NX_Vec3 NX_GetLightDirection(const NX_Light* light)
{
    NX_Vec3 result = NX_VEC3_ZERO;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            result = dir.direction;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = spot.direction;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_LOG(W, "RENDER: Cannot retrieve direction of an omni-directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightDirection(NX_Light* light, NX_Vec3 direction)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            dir.direction = NX_Vec3Normalize(direction);
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.direction = NX_Vec3Normalize(direction);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_LOG(W, "RENDER: Cannot assign direction to an omni-directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

NX_Color NX_GetLightColor(const NX_Light* light)
{
    NX_Color result = NX_BLACK;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            result.r = dir.color.x;
            result.g = dir.color.y;
            result.b = dir.color.z;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result.r = spot.color.x;
            result.g = spot.color.y;
            result.b = spot.color.z;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            result.r = omni.color.x;
            result.g = omni.color.y;
            result.b = omni.color.z;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightColor(NX_Light* light, NX_Color color)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            dir.color = NX_VEC3(color.r, color.g, color.b);
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.color = NX_VEC3(color.r, color.g, color.b);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            omni.color = NX_VEC3(color.r, color.g, color.b);
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

float NX_GetLightEnergy(const NX_Light* light)
{
    float result = 0.0f;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            result = dir.energy;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = spot.energy;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            result = omni.energy;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightEnergy(NX_Light* light, float energy)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            dir.energy = energy;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.energy = energy;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            omni.energy = energy;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

float NX_GetLightSpecular(const NX_Light* light)
{
    float result = 0.0f;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            result = dir.specular;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = spot.specular;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            result = omni.specular;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightSpecular(NX_Light* light, float specular)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            dir.specular = specular;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.specular = specular;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            omni.specular = specular;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

float NX_GetLightRange(const NX_Light* light)
{
    float result = 0.0f;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            result = dir.range; //< Only used for shadow projection
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = spot.range;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            result = omni.range;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightRange(NX_Light* light, float range)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            dir.range = range;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.range = range;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            omni.range = range;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

float NX_GetLightAttenuation(const NX_Light* light)
{
    float result = 0.0f;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot retrieve attenuation of a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = spot.attenuation;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            const INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            result = omni.attenuation;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightAttenuation(NX_Light* light, float attenuation)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot assign attenuation to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.attenuation = attenuation;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            INX_OmniLight& omni = std::get<INX_OmniLight>(light->data);
            omni.attenuation = attenuation;
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

float NX_GetLightInnerCutOff(const NX_Light* light)
{
    float result = 0.0f;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot retrieve inner cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = std::acos(spot.innerCutOff);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_LOG(W, "RENDER: Cannot retrieve inner cutoff to an omni-directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightInnerCutOff(NX_Light* light, float radians)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot assign inner cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.innerCutOff = std::cos(radians);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_LOG(W, "RENDER: Cannot assign inner cutoff to an omni-directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

float NX_GetLightOuterCutOff(const NX_Light* light)
{
    float result = 0.0f;

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot retrieve outer cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            const INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            result = std::acos(spot.outerCutOff);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_LOG(W, "RENDER: Cannot retrieve outer cutoff to an omni-directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }

    return result;
}

void NX_SetLightOuterCutOff(NX_Light* light, float radians)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot assign outer cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.outerCutOff = std::cos(radians);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_LOG(W, "RENDER: Cannot assign outer cutoff to an omni-directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

void NX_SetLightCutOff(NX_Light* light, float inner, float outer)
{
    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            NX_LOG(W, "RENDER: Cannot assign cutoff to a directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_SPOT:
        {
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.innerCutOff = std::cos(inner);
            spot.outerCutOff = std::cos(outer);
        }
        break;
    case NX_LIGHT_OMNI:
        {
            NX_LOG(W, "RENDER: Cannot assign cutoff to an omni-directional light (operation ignored)");
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

bool NX_IsShadowActive(const NX_Light* light)
{
    return light->shadow.active;
}

void NX_SetShadowActive(NX_Light* light, bool active)
{
    if (light->shadow.active == active) {
        return;
    }

    if (active) {
        light->shadow.state.mapIndex = INX_Render3DState_RequestShadowMap(light->type);
    }
    else {
        INX_Render3DState_ReleaseShadowMap(light->type, light->shadow.state.mapIndex);
        light->shadow.state.mapIndex = -1;
    }

    light->shadow.active = active;
}

NX_Layer NX_GetShadowCullMask(const NX_Light* light)
{
    return light->shadow.cullMask;
}

void NX_SetShadowCullMask(NX_Light* light, NX_Layer layers)
{
    light->shadow.cullMask = layers;
}

float NX_GetShadowSlopeBias(NX_Light* light)
{
    return light->shadow.data.slopeBias;
}

void NX_SetShadowSlopeBias(NX_Light* light, float slopeBias)
{
    light->shadow.data.slopeBias = slopeBias;
}

float NX_GetShadowBias(NX_Light* light)
{
    return light->shadow.data.bias;
}

void NX_SetShadowBias(NX_Light* light, float bias)
{
    light->shadow.data.bias = bias;
}

float NX_GetShadowSoftness(const NX_Light* light)
{
    return light->shadow.data.softness;
}

void NX_SetShadowSoftness(NX_Light* light, float softness)
{
    light->shadow.data.softness = softness;
}

float NX_GetShadowOpacity(const NX_Light* light)
{
    return light->shadow.data.opacity;
}

void NX_SetShadowOpacity(NX_Light* light, float opacity)
{
    light->shadow.data.opacity = opacity;
}
