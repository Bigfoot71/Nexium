/* ViewFrustum.hpp -- Class representing the frustum and all associated data for the main view point of the scene
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_SCENE_VIEW_FRUSTUM_HPP
#define HP_SCENE_VIEW_FRUSTUM_HPP

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Math.h>

#include "../../Detail/GPU/Buffer.hpp"
#include "./Frustum.hpp"

namespace scene {

/* === Declaration === */

class ViewFrustum : public Frustum {
public:
    ViewFrustum();

    /** ViewFrustum update */
    void update(const HP_Camera& camera, float aspect);

    /** Layer culling */
    HP_Layer cullMask() const;

    /** Distance to view */
    float getDistanceSquaredTo(const HP_Vec3& point) const;
    float getDistanceSquaredToFarthestPoint(const HP_BoundingBox& box, const HP_Mat4& transform) const;

    /** Matrices */
    const HP_Vec3& viewPosition() const;
    const HP_Mat4& viewProj() const;
    const HP_Mat4& invView() const;
    const HP_Mat4& invProj() const;
    const HP_Mat4& view() const;
    const HP_Mat4& proj() const;

    /** Projection */
    float near() const;
    float far() const;

    /** ViewFrustum UBO */
    const gpu::Buffer& buffer() const;

private:
    struct GPUData {
        alignas(16) HP_Mat4 viewProj{};
        alignas(16) HP_Mat4 view{};
        alignas(16) HP_Mat4 proj{};
        alignas(16) HP_Mat4 invViewProj{};
        alignas(16) HP_Mat4 invView{};
        alignas(16) HP_Mat4 invProj{};
        alignas(16) HP_Vec3 position{};
        alignas(4) uint32_t cullMask{};
        alignas(4) float near;
        alignas(4) float far;
    };

private:
    gpu::Buffer mUniform{};     //< Uniform buffer that stores data
    GPUData mData{};            //< Data uploaded to the uniform buffer
};

/* === Public Implementation === */

inline ViewFrustum::ViewFrustum()
    : mUniform(GL_UNIFORM_BUFFER, sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW)
{ }

inline void ViewFrustum::update(const HP_Camera& camera, float aspect)
{
    /* --- Save raw data from camera --- */

    mData.position = camera.position;
    mData.cullMask = camera.cullMask;
    mData.near = camera.nearPlane;
    mData.far = camera.farPlane;

    /* --- Compute view matrix --- */

    HP_Mat4 T = HP_Mat4Translate(-camera.position);
    HP_Mat4 R = HP_QuatToMat4(camera.rotation);
    R = HP_Mat4Transpose(&R);

    mData.view = T * R;

    /* --- Compute projection matrix --- */

    switch (camera.projection) {
    case HP_PROJECTION_PERSPECTIVE:
        {
            float top = camera.nearPlane * tanf(camera.fov * 0.5f);
            float right = top * aspect;
            mData.proj = HP_Mat4Frustum(-right, right, -top, top, camera.nearPlane, camera.farPlane);
        }
        break;
    case HP_PROJECTION_ORTHOGRAPHIC:
        {
            float top = camera.fov * 0.5f;
            float right = top * aspect;
            mData.proj = HP_Mat4Ortho(-right, right, -top, top, camera.nearPlane, camera.farPlane);
        }
        break;
    }

    /* --- Compute view/proj matrix --- */

    mData.viewProj = mData.view * mData.proj;

    /* --- Compute inverse matrices --- */

    mData.invViewProj = HP_Mat4Inverse(&mData.viewProj);
    mData.invView = HP_Mat4Inverse(&mData.view);
    mData.invProj = HP_Mat4Inverse(&mData.proj);

    /* --- Compute frustum planes --- */

    Frustum::update(mData.viewProj);

    /* --- Upload to the uniform buffer --- */

    mUniform.upload(&mData);
}

inline HP_Layer ViewFrustum::cullMask() const
{
    return mData.cullMask;
}

inline float ViewFrustum::getDistanceSquaredTo(const HP_Vec3& point) const
{
    return HP_Vec3DistanceSq(mData.position, point);
}

inline float ViewFrustum::getDistanceSquaredToFarthestPoint(const HP_BoundingBox& box, const HP_Mat4& transform) const
{
    float maxDistSq = 0.0f;

    for (int x = 0; x <= 1; ++x)
     for (int y = 0; y <= 1; ++y)
      for (int z = 0; z <= 1; ++z)
    {
        HP_Vec3 corner = {
            x ? box.max.x : box.min.x,
            y ? box.max.y : box.min.y,
            z ? box.max.z : box.min.z
        };

        float distSq = HP_Vec3DistanceSq(
            mData.position, corner * transform
        );

        if (distSq > maxDistSq) maxDistSq = distSq;
    }

    return maxDistSq;
}

inline const HP_Vec3& ViewFrustum::viewPosition() const
{
    return mData.position;
}

inline const HP_Mat4& ViewFrustum::viewProj() const
{
    return mData.viewProj;
}

inline const HP_Mat4& ViewFrustum::invView() const
{
    return mData.invView;
}

inline const HP_Mat4& ViewFrustum::invProj() const
{
    return mData.invProj;
}

inline const HP_Mat4& ViewFrustum::view() const
{
    return mData.view;
}

inline const HP_Mat4& ViewFrustum::proj() const
{
    return mData.proj;
}

inline float ViewFrustum::near() const
{
    return mData.near;
}

inline float ViewFrustum::far() const
{
    return mData.far;
}

inline const gpu::Buffer& ViewFrustum::buffer() const
{
    return mUniform;
}

} // namespace scene

#endif // HP_SCENE_VIEW_FRUSTUM_HPP
