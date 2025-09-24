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

#ifndef HP_SCENE_FRUSTUM_HPP
#define HP_SCENE_FRUSTUM_HPP

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Math.h>

namespace scene {

/* === Declaration === */

class Frustum {
public:
    enum Plane : uint32_t {
        Back, Front,
        Bottom, Top,
        Right, Left,
        PLANE_COUNT
    };

public:
    Frustum() = default;

    /** Frustum update */
    void update(const HP_Mat4& viewProj);

    /** Frustum culling */
    bool containsPoint(const HP_Vec3& position) const;
    bool containsPoints(const HP_Vec3* positions, int count) const;
    bool cointainsSphere(const HP_Vec3& position, float radius) const;
    bool containsAabb(const HP_BoundingBox& aabb) const;
    bool containsObb(const HP_BoundingBox& aabb, const HP_Transform& transform) const;

private:
    /** Helper functions */
    static float distanceToPlane(const HP_Vec4& plane, const HP_Vec3& position);

private:
    HP_Vec4 mPlanes[6]{};
};

/* === Public Implementation === */

inline void Frustum::update(const HP_Mat4& viewProj)
{
    mPlanes[Right] = HP_Vec4Normalize({
        viewProj.m03 - viewProj.m00,
        viewProj.m13 - viewProj.m10,
        viewProj.m23 - viewProj.m20,
        viewProj.m33 - viewProj.m30
    });

    mPlanes[Left] = HP_Vec4Normalize({
        viewProj.m03 + viewProj.m00,
        viewProj.m13 + viewProj.m10,
        viewProj.m23 + viewProj.m20,
        viewProj.m33 + viewProj.m30
    });

    mPlanes[Top] = HP_Vec4Normalize({
        viewProj.m03 - viewProj.m01,
        viewProj.m13 - viewProj.m11,
        viewProj.m23 - viewProj.m21,
        viewProj.m33 - viewProj.m31
    });

    mPlanes[Bottom] = HP_Vec4Normalize({
        viewProj.m03 + viewProj.m01,
        viewProj.m13 + viewProj.m11,
        viewProj.m23 + viewProj.m21,
        viewProj.m33 + viewProj.m31
    });

    mPlanes[Back] = HP_Vec4Normalize({
        viewProj.m03 - viewProj.m02,
        viewProj.m13 - viewProj.m12,
        viewProj.m23 - viewProj.m22,
        viewProj.m33 - viewProj.m32
    });

    mPlanes[Front] = HP_Vec4Normalize({
        viewProj.m03 + viewProj.m02,
        viewProj.m13 + viewProj.m12,
        viewProj.m23 + viewProj.m22,
        viewProj.m33 + viewProj.m32
    });
}

inline bool Frustum::containsPoint(const HP_Vec3& position) const
{
    for (int i = 0; i < PLANE_COUNT; i++) {
        if (distanceToPlane(mPlanes[i], position) <= 0) {
            return false;
        }
    }
    return true;
}

inline bool Frustum::containsPoints(const HP_Vec3* positions, int count) const
{
    for (int i = 0; i < count; i++) {
        if (containsPoint(positions[i])) {
            return true;
        }
    }
    return false;
}

inline bool Frustum::cointainsSphere(const HP_Vec3& position, float radius) const
{
    for (int i = 0; i < PLANE_COUNT; i++) {
        if (distanceToPlane(mPlanes[i], position) < -radius) {
            return false;
        }
    }
    return true;
}

inline bool Frustum::containsAabb(const HP_BoundingBox& aabb) const
{
    float xMin = aabb.min.x, yMin = aabb.min.y, zMin = aabb.min.z;
    float xMax = aabb.max.x, yMax = aabb.max.y, zMax = aabb.max.z;

    for (int i = 0; i < PLANE_COUNT; i++)
    {
        const HP_Vec4& plane = mPlanes[i];

        // Choose the optimal coordinates according to the sign of the normal
        float distance = distanceToPlane(mPlanes[i], HP_Vec3 {
            .x = (plane.x >= 0.0f) ? xMax : xMin,
            .y = (plane.y >= 0.0f) ? yMax : yMin,
            .z = (plane.z >= 0.0f) ? zMax : zMin
        });

        if (distance < -1e-6f) {
            return false;
        }
    }
    return true;
}

inline bool Frustum::containsObb(const HP_BoundingBox& aabb, const HP_Transform& transform) const
{
    /* --- Compute OBB center and extents in local space --- */

    HP_Vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
    HP_Vec3 extents = (aabb.max - aabb.min) * 0.5f;

    /* --- Transform center to world space --- */

    HP_Vec3 scaledCenter = localCenter * transform.scale;
    HP_Vec3 rotatedCenter = HP_Vec3Rotate(scaledCenter, transform.rotation);
    HP_Vec3 worldCenter = rotatedCenter + transform.translation;

    /* --- Compute the 3 OBB axes (columns of rotation matrix scaled) --- */

    HP_Vec3 obbAxisX = HP_Vec3Rotate(HP_VEC3(transform.scale.x, 0, 0), transform.rotation);
    HP_Vec3 obbAxisY = HP_Vec3Rotate(HP_VEC3(0, transform.scale.y, 0), transform.rotation);
    HP_Vec3 obbAxisZ = HP_Vec3Rotate(HP_VEC3(0, 0, transform.scale.z), transform.rotation);

    /* --- Test OBB against each frustum plane --- */

    for (int i = 0; i < PLANE_COUNT; i++)
    {
        const HP_Vec4& plane = mPlanes[i];

        // Signed distance from OBB center to plane
        float centerDistance = distanceToPlane(plane, worldCenter);

        // Project OBB extents onto plane normal
        float projectedRadius =
            fabsf(HP_Vec3Dot(HP_VEC3(plane.x, plane.y, plane.z), obbAxisX)) * extents.x +
            fabsf(HP_Vec3Dot(HP_VEC3(plane.x, plane.y, plane.z), obbAxisY)) * extents.y +
            fabsf(HP_Vec3Dot(HP_VEC3(plane.x, plane.y, plane.z), obbAxisZ)) * extents.z;

        if (centerDistance + projectedRadius < -1e-6f) {
            return false;
        }
    }

    return true;
}

/* === Private Implementation === */

inline float Frustum::distanceToPlane(const HP_Vec4& plane, const HP_Vec3& position)
{
    return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
}

} // namespace scene

#endif // HP_SCENE_FRUSTUM_HPP
