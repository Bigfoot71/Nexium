#ifndef NX_SCENE_CULLING_HPP
#define NX_SCENE_CULLING_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Math.h>
#include <array>

namespace scene {

/* === Oriented Bounding Box === */

struct OrientedBoundingBox {
    NX_Vec3 center;                 // center world-space
    std::array<NX_Vec3, 3> axes;    // axes en world-space, longueur = scale
    NX_Vec3 extents;                // extents locaux

    OrientedBoundingBox(const NX_BoundingBox& aabb, const NX_Transform& transform);
};

inline OrientedBoundingBox::OrientedBoundingBox(const NX_BoundingBox& aabb, const NX_Transform& transform)
{
    NX_Vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
    NX_Vec3 scaledCenter = localCenter * transform.scale;
    this->center = NX_Vec3Rotate(scaledCenter, transform.rotation) + transform.translation;
    this->axes[0] = NX_Vec3Rotate(NX_VEC3(transform.scale.x, 0, 0), transform.rotation);
    this->axes[1] = NX_Vec3Rotate(NX_VEC3(0, transform.scale.y, 0), transform.rotation);
    this->axes[2] = NX_Vec3Rotate(NX_VEC3(0, 0, transform.scale.z), transform.rotation);
    this->extents = (aabb.max - aabb.min) * 0.5f;

}

/* === Bounding Sphere === */

struct BoundingSphere {
    NX_Vec3 center;
    float radius;

    BoundingSphere(const NX_BoundingBox& aabb, const NX_Transform& transform);
};

inline BoundingSphere::BoundingSphere(const NX_BoundingBox& aabb, const NX_Transform& transform)
{
    NX_Vec3 localCenter = (aabb.min + aabb.max) * 0.5f;
    NX_Vec3 rotatedCenter = localCenter * transform.rotation;
    this->center = transform.translation + rotatedCenter;

    NX_Vec3 halfSize = (aabb.max - aabb.min) * 0.5f;
    this->radius = NX_Vec3Length(halfSize * transform.scale);
}

} // namespace scene

#endif // NX_SCENE_CULLING_HPP
