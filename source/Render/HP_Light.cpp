/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "./HP_Light.hpp"

#include "./Scene/LightManager.hpp"
#include "./Scene/DrawCall.hpp"
#include "./Scene/DrawData.hpp"
#include "Hyperion/HP_Render.h"

/* === Public Implementation === */

HP_Light::HP_Light(scene::LightManager& manager, HP_LightType type)
    : mManager(manager)
    , mData(Directional())
    , mType(type)
{
    switch (type) {
    case HP_LIGHT_DIR:
        mShadowData.lambda = 60;
        mData = Directional();
        break;
    case HP_LIGHT_SPOT:
        mShadowData.lambda = 40;
        mData = Spot();
        break;
    case HP_LIGHT_OMNI:
        mShadowData.lambda = 40;
        mData = Omni();
        break;
    default:
        HP_INTERNAL_LOG(W, "RENDER: Invalid light type (%i); The light will be invalid");
        break;
    }

    mShadowData.softness = 1.0f / manager.shadowResolution();
    mShadowData.bleedingBias = 0.2f;
}

/* === Private Implementation === */

void HP_Light::markDirty(bool light, bool shadow, bool viewProj)
{
    if (viewProj) mShadowState.vpDirty = true;
    if (shadow) mManager.markShadowDirty();
    if (light) mManager.markLightDirty();
}

void HP_Light::updateDirectionalViewProj(const HP_BoundingBox& sceneBounds)
{
    SDL_assert(mType == HP_LIGHT_DIR);
    SDL_assert(mHasShadow);

    Directional& light = std::get<Directional>(mData);

    /* --- Calculating the center and extents of the scene --- */

    constexpr float sceneMargin = 1.1f; // 10% margin
    HP_Vec3 sceneCenter = (sceneBounds.min + sceneBounds.max) * 0.5f;
    HP_Vec3 sceneExtents = (sceneBounds.max - sceneBounds.min) * 0.5f * sceneMargin;

    /* --- Normalizing the light direction --- */

    HP_Vec3 lightDir = HP_Vec3Normalize(light.direction);

    /* --- Calculating the light position (placed at a distance from the center of the scene) --- */

    float maxSceneExtent = fmaxf(sceneExtents.x, fmaxf(sceneExtents.y, sceneExtents.z));
    float lightDistance = 2.0f * maxSceneExtent;
    HP_Vec3 pos = sceneCenter + (-lightDir * lightDistance);

    /* --- Calculating the view matrix with a stable up vector --- */

    // If the direction is nearly vertical, use Z as the "up" vector
    HP_Vec3 upVector = (fabsf(lightDir.y) > 0.99f) ? HP_VEC3_FORWARD : HP_VEC3_UP;
    HP_Mat4 view = HP_Mat4LookAt(pos, sceneCenter, upVector);

    /* --- Calculating the bounding volume of the scene in light space --- */

    HP_Vec3 corners[8] = {
        {sceneBounds.min.x, sceneBounds.min.y, sceneBounds.min.z},
        {sceneBounds.max.x, sceneBounds.min.y, sceneBounds.min.z},
        {sceneBounds.min.x, sceneBounds.max.y, sceneBounds.min.z},
        {sceneBounds.max.x, sceneBounds.max.y, sceneBounds.min.z},
        {sceneBounds.min.x, sceneBounds.min.y, sceneBounds.max.z},
        {sceneBounds.max.x, sceneBounds.min.y, sceneBounds.max.z},
        {sceneBounds.min.x, sceneBounds.max.y, sceneBounds.max.z},
        {sceneBounds.max.x, sceneBounds.max.y, sceneBounds.max.z}
    };

    float minX = INFINITY, maxX = -INFINITY;
    float minY = INFINITY, maxY = -INFINITY;
    float minZ = INFINITY, maxZ = -INFINITY;

    for (int i = 0; i < 8; i++) {
        HP_Vec3 transformed = corners[i] * view;
        minX = fminf(minX, transformed.x);
        maxX = fmaxf(maxX, transformed.x);
        minY = fminf(minY, transformed.y);
        maxY = fmaxf(maxY, transformed.y);
        minZ = fminf(minZ, transformed.z);
        maxZ = fmaxf(maxZ, transformed.z);
    }

    /* --- Creating the orthographic projection matrix --- */

    // WARNING: In camera space, objects in front of the camera have negative Z values.
    // Here, maxZ corresponds to the closest plane (less negative) and minZ to the farthest plane.
    // To obtain positive distances for the projection, we reverse the signs:
    // near = -maxZ and far = -minZ (which guarantees near < far).

    HP_Mat4 proj = HP_Mat4Ortho(minX, maxX, minY, maxY, -maxZ, -minZ);

    /* --- Keeps useful values ​​and returns the projection view matrix --- */

    mShadowData.viewProj[0] = view * proj;
    light.position = pos;
    light.range = -minZ;

    /* --- Update frustum --- */

    mShadowData.frustum[0].update(mShadowData.viewProj[0]);
}

void HP_Light::updateSpotViewProj()
{
    SDL_assert(mType == HP_LIGHT_SPOT);
    SDL_assert(mHasShadow);

    const Spot& light = std::get<Spot>(mData);

    /* --- Calculate view projection matrix --- */

    HP_Mat4 view = HP_Mat4LookAt(light.position, light.position + light.direction, HP_VEC3_UP);
    HP_Mat4 proj = HP_Mat4Perspective(HP_PI / 2.0f, 1.0f, 0.05f, light.range);

    mShadowData.viewProj[0] = view * proj;

    /* --- Update frustum --- */

    mShadowData.frustum[0].update(mShadowData.viewProj[0]);
}

void HP_Light::updateOmniViewProj()
{
    SDL_assert(mType == HP_LIGHT_OMNI);
    SDL_assert(mHasShadow);

    const Omni& omni = std::get<Omni>(mData);

    /* --- Calculate view projection matrices and frustums --- */

    for (int i = 0; i < mShadowData.viewProj.size(); i++)
    {
        mShadowData.viewProj[i] =
            render::getCubeView(i, omni.position) *
            render::getCubeProj(0.05f, omni.range);

        mShadowData.frustum[i].update(mShadowData.viewProj[i]);
    }
}
