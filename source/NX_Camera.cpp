/* NX_Camera.c -- API definition for Nexium's camera module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Camera.h>
#include <NX/NX_Math.h>
#include <cmath>

// ============================================================================
// LOCAL MANAGEMENT
// ============================================================================

static NX_Camera INX_DefaultCamera = NX_BASE_CAMERA;

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Camera NX_GetDefaultCamera()
{
    return INX_DefaultCamera;
}

void NX_SetDefaultCamera(const NX_Camera* camera)
{
    INX_DefaultCamera = camera ? *camera : NX_BASE_CAMERA;
}

void NX_UpdateCameraOrbital(NX_Camera* camera, NX_Vec3 center, float distance, float height, float rotation)
{
    camera->position.x = center.x + distance * std::cos(rotation);
    camera->position.z = center.z + distance * std::sin(rotation);
    camera->position.y = center.y + height;

    camera->rotation = NX_QuatLookAt(camera->position, center, NX_VEC3_UP);
}

void NX_UpdateCameraFree(NX_Camera* camera, NX_Vec3 movement, NX_Vec3 rotation, float maxPitch)
{
    /* --- Rotation (Euler) --- */

    NX_Vec3 euler = NX_QuatToEuler(camera->rotation);

    euler.x += rotation.x;
    euler.y += rotation.y;
    euler.z += rotation.z;

    if (maxPitch < 0.0f) maxPitch = NX_PI * 0.49f;
    euler.x = NX_CLAMP(euler.x, -maxPitch, maxPitch);
    euler.y = NX_WrapRadians(euler.y);
    euler.z = NX_WrapRadians(euler.z);

    camera->rotation = NX_QuatFromEuler(euler);

    /* --- Translation --- */

    NX_Vec3 forward = NX_Vec3Rotate(NX_VEC3_FORWARD, camera->rotation);
    NX_Vec3 right   = NX_Vec3Rotate(NX_VEC3_RIGHT,   camera->rotation);
    NX_Vec3 up      = NX_Vec3Rotate(NX_VEC3_UP,      camera->rotation);

    NX_Vec3 deltaPos = NX_VEC3_ZERO;
    deltaPos = NX_Vec3MulAdd(forward, -movement.z, deltaPos);
    deltaPos = NX_Vec3MulAdd(right,    movement.x, deltaPos);
    deltaPos = NX_Vec3MulAdd(up,       movement.y, deltaPos);

    camera->position = NX_Vec3Add(camera->position, deltaPos);
}

void NX_UpdateCameraFPS(NX_Camera* camera, NX_Vec3 movement, NX_Vec3 rotation, float maxPitch)
{
    /* --- Rotation (Euler) --- */

    NX_Vec3 euler = NX_QuatToEuler(camera->rotation);

    euler.x += rotation.x;
    euler.y += rotation.y;
    euler.z += rotation.z;

    if (maxPitch < 0.0f) maxPitch = NX_PI * 0.49f;
    euler.x = NX_CLAMP(euler.x, -maxPitch, maxPitch);
    euler.y = NX_WrapRadians(euler.y);
    euler.z = NX_WrapRadians(euler.z);

    camera->rotation = NX_QuatFromEuler(euler);

    /* --- Translation --- */

    NX_Quat yawOnly = NX_QuatFromEuler(NX_VEC3(0.0f, euler.y, 0.0f));

    NX_Vec3 forward = NX_Vec3Rotate(NX_VEC3_FORWARD, yawOnly);
    NX_Vec3 right   = NX_Vec3Rotate(NX_VEC3_RIGHT,   yawOnly);
    NX_Vec3 up      = NX_VEC3_UP;

    NX_Vec3 deltaPos = NX_VEC3_ZERO;
    deltaPos = NX_Vec3MulAdd(forward, -movement.z, deltaPos);
    deltaPos = NX_Vec3MulAdd(right,    movement.x, deltaPos);
    deltaPos = NX_Vec3MulAdd(up,       movement.y, deltaPos);

    camera->position = NX_Vec3Add(camera->position, deltaPos);
}

void NX_ApplyCameraTransform(NX_Camera* camera, NX_Mat4 transform, NX_Vec3 offset)
{
    camera->rotation = NX_QuatFromMat4(&transform);

    NX_Vec3 transformPosition = NX_VEC3(transform.v[2][0], transform.v[2][1], transform.v[2][2]);
    NX_Vec3 rotatedOffset = NX_Vec3TransformByMat4(offset, &transform);

    camera->position = NX_Vec3Add(transformPosition, rotatedOffset);
}

NX_Transform NX_GetCameraTransform(const NX_Camera* camera)
{
    return NX_TRANSFORM_T {
        .translation = camera->position,
        .rotation = camera->rotation,
        .scale = NX_VEC3_ONE
    };
}

NX_Mat4 NX_GetCameraViewMatrix(const NX_Camera* camera)
{
    // Equivalent to:
    //  translate(-position) * transpose(mat4(rotation))

    float a2 = camera->rotation.x * camera->rotation.x;
    float b2 = camera->rotation.y * camera->rotation.y;
    float c2 = camera->rotation.z * camera->rotation.z;
    float ac = camera->rotation.x * camera->rotation.z;
    float ab = camera->rotation.x * camera->rotation.y;
    float bc = camera->rotation.y * camera->rotation.z;
    float ad = camera->rotation.w * camera->rotation.x;
    float bd = camera->rotation.w * camera->rotation.y;
    float cd = camera->rotation.w * camera->rotation.z;

    NX_Mat4 view;

    view.m00 = 1 - 2 * (b2 + c2);
    view.m01 = 2 * (ab - cd);
    view.m02 = 2 * (ac + bd);
    view.m03 = 0.0f;

    view.m10 = 2 * (ab + cd);
    view.m11 = 1 - 2 * (a2 + c2);
    view.m12 = 2 * (bc - ad);
    view.m13 = 0.0f;

    view.m20 = 2 * (ac - bd);
    view.m21 = 2 * (bc + ad);
    view.m22 = 1 - 2 * (a2 + b2);
    view.m23 = 0.0f;

    view.m30 = -(view.m00 * camera->position.x + view.m10 * camera->position.y + view.m20 * camera->position.z);
    view.m31 = -(view.m01 * camera->position.x + view.m11 * camera->position.y + view.m21 * camera->position.z);
    view.m32 = -(view.m02 * camera->position.x + view.m12 * camera->position.y + view.m22 * camera->position.z);
    view.m33 = 1.0f;

    return view;
}

NX_Mat4 NX_GetCameraProjectionMatrix(const NX_Camera* camera, float aspect)
{
    switch (camera->projection) {
    case NX_PROJECTION_PERSPECTIVE:
        {
            float top = camera->nearPlane * tanf(camera->fov * 0.5f);
            float right = top * aspect;
            return NX_Mat4Frustum(-right, right, -top, top, camera->nearPlane, camera->farPlane);
        }
        break;
    case NX_PROJECTION_ORTHOGRAPHIC:
        {
            float top = camera->fov * 0.5f;
            float right = top * aspect;
            return NX_Mat4Ortho(-right, right, -top, top, camera->nearPlane, camera->farPlane);
        }
        break;
    default:
        break;
    }

    return NX_MAT4_IDENTITY;
}
