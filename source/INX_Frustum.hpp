/* INX_Frustum.hpp -- Base class representing a frustum
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef INX_FRUSTUM_HPP
#define INX_FRUSTUM_HPP

#include "./NX_Shape.hpp"
#include <NX/NX_Math.h>
#include <array>

/* === Declaration === */

class INX_Frustum {
public:
    enum Plane : uint32_t {
        Back, Front,
        Bottom, Top,
        Right, Left,
        PLANE_COUNT
    };

    enum Containment : uint8_t {
        Outside, Inside, Intersect
    };

public:
    /** Constructors */
    INX_Frustum() = default;
    INX_Frustum(const NX_Mat4& viewProj);

    /** INX_Frustum update */
    void Update(const NX_Mat4& viewProj);

    /** Contains or not */
    bool ContainsPoint(const NX_Vec3& position) const;
    bool ContainsPoints(const NX_Vec3* positions, int count) const;
    bool ContainsSphere(const INX_BoundingSphere3D& sphere) const;
    bool ContainsAabb(const NX_BoundingBox3D& aabb) const;
    bool ContainsObb(const INX_OrientedBoundingBox3D& obb) const;

    /** Classification */
    Containment ClassifySphere(const INX_BoundingSphere3D& sphere) const;

private:
    /** Helper functions */
    static float DistanceToPlane(const NX_Vec4& plane, const NX_Vec3& position);

private:
    NX_Vec4 mPlanes[6]{};
};

/* === Public Implementation === */

inline INX_Frustum::INX_Frustum(const NX_Mat4& viewProj)
{
    this->Update(viewProj);
}

inline void INX_Frustum::Update(const NX_Mat4& viewProj)
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
}

inline bool INX_Frustum::ContainsPoint(const NX_Vec3& position) const
{
    for (int i = 0; i < PLANE_COUNT; i++) {
        if (DistanceToPlane(mPlanes[i], position) <= 0) {
            return false;
        }
    }
    return true;
}

inline bool INX_Frustum::ContainsPoints(const NX_Vec3* positions, int count) const
{
    for (int i = 0; i < count; i++) {
        if (ContainsPoint(positions[i])) {
            return true;
        }
    }
    return false;
}

inline bool INX_Frustum::ContainsSphere(const INX_BoundingSphere3D& sphere) const
{
    for (int i = 0; i < PLANE_COUNT; i++) {
        if (DistanceToPlane(mPlanes[i], sphere.center) < -sphere.radius) {
            return false;
        }
    }
    return true;
}

inline bool INX_Frustum::ContainsAabb(const NX_BoundingBox3D& aabb) const
{
    float xMin = aabb.min.x, yMin = aabb.min.y, zMin = aabb.min.z;
    float xMax = aabb.max.x, yMax = aabb.max.y, zMax = aabb.max.z;

    for (int i = 0; i < PLANE_COUNT; i++)
    {
        const NX_Vec4& plane = mPlanes[i];

        // Choose the optimal coordinates according to the sign of the normal
        float distance = DistanceToPlane(mPlanes[i], NX_Vec3 {
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

inline bool INX_Frustum::ContainsObb(const INX_OrientedBoundingBox3D& obb) const
{
    for (int i = 0; i < PLANE_COUNT; i++)
    {
        const NX_Vec4& plane = mPlanes[i];

        float centerDistance = DistanceToPlane(plane, obb.center);

        float projectedRadius =
            std::abs(NX_Vec3Dot(NX_VEC3(plane.x, plane.y, plane.z), obb.axes[0])) * obb.extents.x +
            std::abs(NX_Vec3Dot(NX_VEC3(plane.x, plane.y, plane.z), obb.axes[1])) * obb.extents.y +
            std::abs(NX_Vec3Dot(NX_VEC3(plane.x, plane.y, plane.z), obb.axes[2])) * obb.extents.z;

        if (centerDistance + projectedRadius < -1e-6f) {
            return false;
        }
    }

    return true;
}

inline INX_Frustum::Containment INX_Frustum::ClassifySphere(const INX_BoundingSphere3D& sphere) const
{
    bool fullyInside = true;

    for (int i = 0; i < PLANE_COUNT; ++i) {
        float d = DistanceToPlane(mPlanes[i], sphere.center);
        if (d < -sphere.radius) return Containment::Outside;
        if (d < sphere.radius) fullyInside = false;
    }

    return fullyInside ? Containment::Inside : Containment::Intersect;
}

/* === Private Implementation === */

inline float INX_Frustum::DistanceToPlane(const NX_Vec4& plane, const NX_Vec3& position)
{
    return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
}

#endif // INX_FRUSTUM_HPP
