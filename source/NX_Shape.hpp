/* NX_Shape.h -- API declaration for Nexium's shape module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SHAPE_HPP
#define NX_SHAPE_HPP

#include <NX/NX_Shape.h>
#include <array>

// ============================================================================
// INTERNAL TYPES
// ============================================================================

struct INX_BoundingSphere3D {
    INX_BoundingSphere3D(const NX_BoundingBox3D& aabb, const NX_Transform& transform);
    NX_Vec3 center;
    float radius;
};

inline INX_BoundingSphere3D::INX_BoundingSphere3D(const NX_BoundingBox3D& aabb, const NX_Transform& transform)
{
    NX_Vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
    NX_Vec3 rotatedCenter = localCenter * transform.rotation;
    this->center = transform.translation + rotatedCenter;

    NX_Vec3 halfSize = (aabb.max - aabb.min) * 0.5f;
    this->radius = NX_Vec3Length(halfSize * transform.scale);
}

struct INX_OrientedBoundingBox3D {
    INX_OrientedBoundingBox3D(const NX_BoundingBox3D& aabb, const NX_Transform& transform);
    std::array<NX_Vec3, 3> axes;    // world-space axes, length = scale
    NX_Vec3 center, extents;        // world-space center and local extents
};

inline INX_OrientedBoundingBox3D::INX_OrientedBoundingBox3D(const NX_BoundingBox3D& aabb, const NX_Transform& transform)
{
    NX_Vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
    NX_Vec3 scaledCenter = localCenter * transform.scale;
    this->center = NX_Vec3Rotate(scaledCenter, transform.rotation) + transform.translation;
    this->axes[0] = NX_Vec3Rotate(NX_VEC3(transform.scale.x, 0, 0), transform.rotation);
    this->axes[1] = NX_Vec3Rotate(NX_VEC3(0, transform.scale.y, 0), transform.rotation);
    this->axes[2] = NX_Vec3Rotate(NX_VEC3(0, 0, transform.scale.z), transform.rotation);
    this->extents = (aabb.max - aabb.min) * 0.5f;
}

#endif // NX_SHAPE_HPP
