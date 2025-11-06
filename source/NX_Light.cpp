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
#include <cmath>

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

static void INX_UpdateDirectionalLight_ViewProj(NX_Light* light, const INX_ViewFrustum& viewFrustum)
{
    SDL_assert(light->type == NX_LIGHT_DIR);
    SDL_assert(light->shadow.active);

    INX_DirectionalLight& dirLight = std::get<INX_DirectionalLight>(light->data);
    const NX_Vec3& cameraPos = viewFrustum.GetViewPosition();
    const NX_Vec3& lightDir = dirLight.direction;

    /* --- Calcuate view matrix --- */

    NX_Vec3 up = (std::abs(NX_Vec3Dot(lightDir, NX_VEC3_UP)) > 0.99f) ? NX_VEC3_BACK : NX_VEC3_UP;
    NX_Mat4 view = NX_Mat4LookTo(cameraPos, lightDir, up);

    /* --- Calculate projection matrix --- */

    NX_Vec3 rightLS = NX_VEC3(view.m00, view.m10, view.m20);
    NX_Vec3 upLS    = NX_VEC3(view.m01, view.m11, view.m21);
    NX_Vec3 forwLS  = NX_VEC3(view.m02, view.m12, view.m22);

    NX_Vec3 extentLS = NX_VEC3(
        std::abs(rightLS.x) + std::abs(upLS.x) + std::abs(forwLS.x),
        std::abs(rightLS.y) + std::abs(upLS.y) + std::abs(forwLS.y),
        std::abs(rightLS.z) + std::abs(upLS.z) + std::abs(forwLS.z)
    ) * dirLight.shadowRadius;

    NX_Mat4 proj = NX_Mat4Ortho(
        -extentLS.x,
        +extentLS.x,
        -extentLS.y,
        +extentLS.y,
        -extentLS.z,
        +extentLS.z
    );

    /* --- Store the results --- */

    light->shadow.data.viewProj[0] = view * proj;

    dirLight.position = cameraPos - lightDir * dirLight.shadowRadius;
    dirLight.range = 2.0f * extentLS.z;

    /* --- Update frustum --- */

    light->shadow.data.frustum[0].Update(light->shadow.data.viewProj[0]);
}

static void INX_UpdateSpotLight_ViewProj(NX_Light* light)
{
    SDL_assert(light->type == NX_LIGHT_SPOT);
    SDL_assert(light->shadow.active);

    const INX_SpotLight& spotLight = std::get<INX_SpotLight>(light->data);

    /* --- Calculate view projection matrix --- */

    constexpr float nearPlane = 0.05f;

    NX_Mat4 view = NX_Mat4LookAt(spotLight.position, spotLight.position + spotLight.direction, NX_VEC3_UP);
    NX_Mat4 proj = NX_Mat4Perspective(NX_PI / 2.0f, 1.0f, nearPlane, nearPlane + spotLight.range);

    light->shadow.data.viewProj[0] = view * proj;

    /* --- Update frustum --- */

    light->shadow.data.frustum[0].Update(light->shadow.data.viewProj[0]);
}

static void INX_UpdateOmniLight_ViewProj(NX_Light* light)
{
    SDL_assert(light->type == NX_LIGHT_OMNI);
    SDL_assert(light->shadow.active);

    const INX_OmniLight& omniLight = std::get<INX_OmniLight>(light->data);

    /* --- Calculate view projection matrices and frustums --- */

    constexpr float nearPlane = 0.05f;

    for (int i = 0; i < light->shadow.data.viewProj.size(); i++)
    {
        light->shadow.data.viewProj[i] =
            INX_GetCubeView(i, omniLight.position) *
            INX_GetCubeProj(nearPlane, nearPlane + omniLight.range);

        light->shadow.data.frustum[i].Update(light->shadow.data.viewProj[i]);
    }
}

void INX_UpdateLight(NX_Light* light, const INX_ViewFrustum& viewFrustum, bool* needsShadowUpdate)
{
    SDL_assert(needsShadowUpdate != nullptr);
    SDL_assert(light->active);

    if (!light->shadow.active) {
        return;
    }

    /* --- Checks if the shadow map needs to be updated --- */

    if (light->shadow.state.forceUpdate) {
        light->shadow.state.forceUpdate = false;
        *needsShadowUpdate = true;
    }

    if (light->shadow.state.updateMode == NX_SHADOW_UPDATE_INTERVAL) {
        light->shadow.state.timerSec += NX_GetDeltaTime();
        if (light->shadow.state.timerSec >= light->shadow.state.intervalSec) {
            light->shadow.state.timerSec -= light->shadow.state.intervalSec;
            *needsShadowUpdate = true;
        }
    }
    else if (light->shadow.state.updateMode == NX_SHADOW_UPDATE_CONTINUOUS) {
        *needsShadowUpdate = true;
    }

    /* --- Update view projection if needed --- */

    switch (light->type) {
    case NX_LIGHT_DIR:
        // NOTE: The view/proj always needs to be updated relative
        //       to the camera if the shadow map needs to be updated
        if (*needsShadowUpdate) {
            INX_UpdateDirectionalLight_ViewProj(light, viewFrustum);
        }
        break;
    case NX_LIGHT_SPOT:
        if (light->shadow.state.vpDirty) {
            light->shadow.state.vpDirty = false;
            INX_UpdateSpotLight_ViewProj(light);
        }
        break;
    case NX_LIGHT_OMNI:
        if (light->shadow.state.vpDirty) {
            light->shadow.state.vpDirty = false;
            INX_UpdateOmniLight_ViewProj(light);
        }
        break;
    case NX_LIGHT_TYPE_COUNT:
        NX_UNREACHABLE();
        break;
    }
}

void INX_FillGPULight(NX_Light* light, INX_GPULight* gpu, int shadowIndex)
{
    SDL_assert(light != nullptr && gpu != nullptr);
    SDL_assert(light->active);

    switch (light->type) {
    case NX_LIGHT_DIR:
        {
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);

            gpu->position = dir.position;
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
    gpu->layerMask = light->layerMask;
}

void INX_FillGPUShadow(NX_Light* light, INX_GPUShadow* gpu, int mapIndex)
{
    SDL_assert(light != nullptr && gpu != nullptr);
    SDL_assert(light->shadow.active);
    SDL_assert(light->active);

    if (light->type != NX_LIGHT_OMNI) {
        gpu->viewProj = light->shadow.data.viewProj[0];
    }

    gpu->mapIndex = mapIndex;
    gpu->slopeBias = light->shadow.data.slopeBias;
    gpu->bias = light->shadow.data.bias;
    gpu->softness = light->shadow.data.softness;
}

const INX_Frustum& INX_GetLightFrustum(const NX_Light& light, int face)
{
    // Assert that:
    // - For non-omni lights, only face 0 is valid.
    // - For omni lights, valid faces are 0 through 5.
    SDL_assert((light.type != NX_LIGHT_OMNI && face == 0) || (light.type == NX_LIGHT_OMNI && face <= 5));

    return light.shadow.data.frustum[face];
}

const NX_Mat4& INX_GetLightViewProj(const NX_Light& light, int face)
{
    // Assert that:
    // - For non-omni lights, only face 0 is valid.
    // - For omni lights, valid faces are 0 through 5.
    SDL_assert((light.type != NX_LIGHT_OMNI && face == 0) || (light.type == NX_LIGHT_OMNI && face <= 5));

    return light.shadow.data.viewProj[face];
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
            const INX_DirectionalLight& dir = std::get<INX_DirectionalLight>(light->data);
            result = dir.position; //< Only used for shadow projection
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
            light->shadow.state.vpDirty = true;
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.position = position;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            light->shadow.state.vpDirty = true;
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
            light->shadow.state.vpDirty = true;
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
            dir.shadowRadius = range;
        }
        break;
    case NX_LIGHT_SPOT:
        {
            light->shadow.state.vpDirty = true;
            INX_SpotLight& spot = std::get<INX_SpotLight>(light->data);
            spot.range = range;
        }
        break;
    case NX_LIGHT_OMNI:
        {
            light->shadow.state.vpDirty = true;
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
            light->shadow.state.vpDirty = true;
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
            light->shadow.state.vpDirty = true;
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

NX_ShadowUpdateMode NX_GetShadowUpdateMode(const NX_Light* light)
{
    return light->shadow.state.updateMode;
}

void NX_SetShadowUpdateMode(NX_Light* light, NX_ShadowUpdateMode mode)
{
    light->shadow.state.updateMode = mode;
}

float NX_GetShadowUpdateInterval(const NX_Light* light)
{
    return light->shadow.state.intervalSec;
}

void NX_SetShadowUpdateInterval(NX_Light* light, float sec)
{
    light->shadow.state.intervalSec = sec;
}

void NX_UpdateShadowMap(NX_Light* light)
{
    light->shadow.state.forceUpdate = true;

    if (light->shadow.state.updateMode == NX_SHADOW_UPDATE_INTERVAL) {
        light->shadow.state.timerSec = 0.0f;
    }
}
