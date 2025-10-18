/* Frustum.hpp -- Base class representing a frustum
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_FRUSTUM_HPP
#define NX_SCENE_FRUSTUM_HPP

#include "../../Detail/Simd.hpp"

#include <NX/NX_Render.h>
#include <NX/NX_Math.h>
#include <array>

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
    void update(const NX_Mat4& viewProj);

    /** Frustum culling */
    bool containsPoint(const NX_Vec3& position) const;
    bool containsPoints(const NX_Vec3* positions, int count) const;
    bool containsSphere(const NX_Vec3& position, float radius) const;
    bool containsAabb(const NX_BoundingBox& aabb) const;
    bool containsObb(const NX_BoundingBox& aabb, const NX_Transform& transform) const;
    bool containsObb(const NX_Vec3& center, const std::array<NX_Vec3, 3>& axes, const NX_Vec3& extents) const;
    simd::Float4 containsObb(const simd::Vec3& center, const std::array<simd::Vec3, 3>& axes, const simd::Vec3& extents) const;

private:
    /** Helper functions */
    static float distanceToPlane(const NX_Vec4& plane, const NX_Vec3& position);
    static simd::Float4 distanceToPlane(const simd::Vec4& plane, const simd::Vec3& position);

private:
    NX_Vec4 mPlanes[6]{};
    simd::Vec4 mVPlanes[6]{};
};

/* === Public Implementation === */

inline void Frustum::update(const NX_Mat4& viewProj)
{
    mPlanes[Right] = NX_Vec4Normalize({
        viewProj.m03 - viewProj.m00,
        viewProj.m13 - viewProj.m10,
        viewProj.m23 - viewProj.m20,
        viewProj.m33 - viewProj.m30
    });

    mPlanes[Left] = NX_Vec4Normalize({
        viewProj.m03 + viewProj.m00,
        viewProj.m13 + viewProj.m10,
        viewProj.m23 + viewProj.m20,
        viewProj.m33 + viewProj.m30
    });

    mPlanes[Top] = NX_Vec4Normalize({
        viewProj.m03 - viewProj.m01,
        viewProj.m13 - viewProj.m11,
        viewProj.m23 - viewProj.m21,
        viewProj.m33 - viewProj.m31
    });

    mPlanes[Bottom] = NX_Vec4Normalize({
        viewProj.m03 + viewProj.m01,
        viewProj.m13 + viewProj.m11,
        viewProj.m23 + viewProj.m21,
        viewProj.m33 + viewProj.m31
    });

    mPlanes[Back] = NX_Vec4Normalize({
        viewProj.m03 - viewProj.m02,
        viewProj.m13 - viewProj.m12,
        viewProj.m23 - viewProj.m22,
        viewProj.m33 - viewProj.m32
    });

    mPlanes[Front] = NX_Vec4Normalize({
        viewProj.m03 + viewProj.m02,
        viewProj.m13 + viewProj.m12,
        viewProj.m23 + viewProj.m22,
        viewProj.m33 + viewProj.m32
    });

    for (int i = 0; i < 6; i++) {
        mVPlanes[i] = simd::Vec4(mPlanes[i]);
    }
}

inline bool Frustum::containsPoint(const NX_Vec3& position) const
{
    for (int i = 0; i < PLANE_COUNT; i++) {
        if (distanceToPlane(mPlanes[i], position) <= 0) {
            return false;
        }
    }
    return true;
}

inline bool Frustum::containsPoints(const NX_Vec3* positions, int count) const
{
    for (int i = 0; i < count; i++) {
        if (containsPoint(positions[i])) {
            return true;
        }
    }
    return false;
}

inline bool Frustum::containsSphere(const NX_Vec3& position, float radius) const
{
    for (int i = 0; i < PLANE_COUNT; i++) {
        if (distanceToPlane(mPlanes[i], position) < -radius) {
            return false;
        }
    }
    return true;
}

inline bool Frustum::containsAabb(const NX_BoundingBox& aabb) const
{
    float xMin = aabb.min.x, yMin = aabb.min.y, zMin = aabb.min.z;
    float xMax = aabb.max.x, yMax = aabb.max.y, zMax = aabb.max.z;

    for (int i = 0; i < PLANE_COUNT; i++)
    {
        const NX_Vec4& plane = mPlanes[i];

        // Choose the optimal coordinates according to the sign of the normal
        float distance = distanceToPlane(mPlanes[i], NX_Vec3 {
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

inline bool Frustum::containsObb(const NX_BoundingBox& aabb, const NX_Transform& transform) const
{
    NX_Vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
    NX_Vec3 extents = (aabb.max - aabb.min) * 0.5f;

    NX_Vec3 scaledCenter = localCenter * transform.scale;
    NX_Vec3 rotatedCenter = NX_Vec3Rotate(scaledCenter, transform.rotation);
    NX_Vec3 worldCenter = rotatedCenter + transform.translation;

    std::array<NX_Vec3, 3> obbAxes;
    obbAxes[0] = NX_Vec3Rotate(NX_VEC3(transform.scale.x, 0, 0), transform.rotation);
    obbAxes[1] = NX_Vec3Rotate(NX_VEC3(0, transform.scale.y, 0), transform.rotation);
    obbAxes[2] = NX_Vec3Rotate(NX_VEC3(0, 0, transform.scale.z), transform.rotation);

    return containsObb(worldCenter, obbAxes, extents);
}

inline bool Frustum::containsObb(const NX_Vec3& center, const std::array<NX_Vec3, 3>& axes, const NX_Vec3& extents) const
{
    for (int i = 0; i < PLANE_COUNT; i++)
    {
        const NX_Vec4& plane = mPlanes[i];

        float centerDistance = distanceToPlane(plane, center);

        float projectedRadius =
            fabsf(NX_Vec3Dot(NX_VEC3(plane.x, plane.y, plane.z), axes[0])) * extents.x +
            fabsf(NX_Vec3Dot(NX_VEC3(plane.x, plane.y, plane.z), axes[1])) * extents.y +
            fabsf(NX_Vec3Dot(NX_VEC3(plane.x, plane.y, plane.z), axes[2])) * extents.z;

        if (centerDistance + projectedRadius < -1e-6f) {
            return false;
        }
    }

    return true;
}

inline simd::Float4 Frustum::containsObb(const simd::Vec3& center, const std::array<simd::Vec3, 3>& axes, const simd::Vec3& extents) const
{
    simd::Float4 mask(-1.0f);

    for (int i = 0; i < PLANE_COUNT; i++)
    {
        const simd::Vec4& plane = mVPlanes[i];

        simd::Float4 centerDistance = distanceToPlane(plane, center);

        simd::Float4 projectedRadius =
            abs(dot(plane, axes[0])) * extents.x() +
            abs(dot(plane, axes[1])) * extents.y() +
            abs(dot(plane, axes[2])) * extents.z();

        mask = mask & (centerDistance + projectedRadius >= simd::Float4(-1e-6f));

        if (simd::movemask(mask) == 0) { // all lanes are false
            break;
        }
    }

    return mask;
}

/* === Private Implementation === */

inline float Frustum::distanceToPlane(const NX_Vec4& plane, const NX_Vec3& position)
{
    return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
}

inline simd::Float4 Frustum::distanceToPlane(const simd::Vec4& plane, const simd::Vec3& position)
{
    return plane.x() * position.x() + plane.y() * position.y() + plane.z() * position.z() + plane.w();
}

} // namespace scene

#endif // NX_SCENE_FRUSTUM_HPP
