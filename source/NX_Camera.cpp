/* NX_Camera.c -- API definition for Nexium's camera module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Camera.h>

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Camera NX_GetDefaultCamera()
{
    return NX_Camera {
        .position = NX_VEC3_ZERO,
        .rotation = NX_QUAT_IDENTITY,
        .nearPlane = 0.05f,
        .farPlane = 4000.0f,
        .fov = 60.0f * NX_DEG2RAD,
        .projection = NX_PROJECTION_PERSPECTIVE,
        .cullMask = NX_LAYER_ALL,
    };
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
