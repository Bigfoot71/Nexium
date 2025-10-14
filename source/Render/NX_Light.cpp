/* NX_Light.cpp -- Implementation of the API for lights
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Light.hpp"

#include "./Scene/LightManager.hpp"

/* === Public Implementation === */

NX_Light::NX_Light(scene::LightManager& manager, NX_LightType type)
    : mManager(manager)
    , mData(Directional())
    , mType(type)
{
    switch (type) {
    case NX_LIGHT_DIR:
        mShadowData.lambda = 60;
        mData = Directional();
        break;
    case NX_LIGHT_SPOT:
        mShadowData.lambda = 40;
        mData = Spot();
        break;
    case NX_LIGHT_OMNI:
        mShadowData.lambda = 40;
        mData = Omni();
        break;
    default:
        NX_INTERNAL_LOG(W, "RENDER: Invalid light type (%i); The light will be invalid");
        break;
    }

    mShadowData.softness = 1.0f / manager.shadowResolution();
    mShadowData.bleedingBias = 0.2f;
}

/* === Private Implementation === */

void NX_Light::updateDirectionalViewProj(const scene::ViewFrustum& viewFrustum)
{
    SDL_assert(mType == NX_LIGHT_DIR);
    SDL_assert(mHasShadow);

    Directional& light = std::get<Directional>(mData);

    const NX_Vec3& cameraPos = viewFrustum.viewPosition();
    const NX_Vec3& lightDir = light.direction;

    /* --- Calcuate view matrix --- */

    NX_Vec3 up = (fabsf(NX_Vec3Dot(lightDir, NX_VEC3_UP)) > 0.99f) ? NX_VEC3_BACK : NX_VEC3_UP;
    NX_Mat4 view = NX_Mat4LookTo(cameraPos, lightDir, up);

    /* --- Calculate projection matrix --- */

    NX_Vec3 rightLS = NX_VEC3(view.m00, view.m10, view.m20);
    NX_Vec3 upLS    = NX_VEC3(view.m01, view.m11, view.m21);
    NX_Vec3 forwLS  = NX_VEC3(view.m02, view.m12, view.m22);

    NX_Vec3 extentLS = NX_VEC3(
        fabsf(rightLS.x) + fabsf(upLS.x) + fabsf(forwLS.x),
        fabsf(rightLS.y) + fabsf(upLS.y) + fabsf(forwLS.y),
        fabsf(rightLS.z) + fabsf(upLS.z) + fabsf(forwLS.z)
    ) * light.shadowRadius;

    NX_Mat4 proj = NX_Mat4Ortho(
        -extentLS.x,
        +extentLS.x,
        -extentLS.y,
        +extentLS.y,
        -extentLS.z,
        +extentLS.z
    );

    /* --- Store the results --- */

    mShadowData.viewProj[0] = view * proj;

    light.position = cameraPos - lightDir * light.shadowRadius;
    light.range = 2.0f * extentLS.z;

    /* --- Update frustum --- */

    mShadowData.frustum[0].update(mShadowData.viewProj[0]);
}

void NX_Light::updateSpotViewProj()
{
    SDL_assert(mType == NX_LIGHT_SPOT);
    SDL_assert(mHasShadow);

    const Spot& light = std::get<Spot>(mData);

    /* --- Calculate view projection matrix --- */

    constexpr float nearPlane = 0.05f;

    NX_Mat4 view = NX_Mat4LookAt(light.position, light.position + light.direction, NX_VEC3_UP);
    NX_Mat4 proj = NX_Mat4Perspective(NX_PI / 2.0f, 1.0f, nearPlane, nearPlane + light.range);

    mShadowData.viewProj[0] = view * proj;

    /* --- Update frustum --- */

    mShadowData.frustum[0].update(mShadowData.viewProj[0]);
}

void NX_Light::updateOmniViewProj()
{
    SDL_assert(mType == NX_LIGHT_OMNI);
    SDL_assert(mHasShadow);

    const Omni& light = std::get<Omni>(mData);

    /* --- Calculate view projection matrices and frustums --- */

    constexpr float nearPlane = 0.05f;

    for (int i = 0; i < mShadowData.viewProj.size(); i++)
    {
        mShadowData.viewProj[i] =
            render::getCubeView(i, light.position) *
            render::getCubeProj(nearPlane, nearPlane + light.range);

        mShadowData.frustum[i].update(mShadowData.viewProj[i]);
    }
}
