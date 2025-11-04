/* INX_ViewFrustum.hpp -- Class representing the frustum and all associated data for the main view point of the scene
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SCENE_VIEW_FRUSTUM_HPP
#define NX_SCENE_VIEW_FRUSTUM_HPP

#include <NX/NX_Camera.h>
#include <NX/NX_Math.h>

#include "./Detail/GPU/Buffer.hpp"
#include "./INX_Frustum.hpp"

/* === Declaration === */

class INX_ViewFrustum : public INX_Frustum {
public:
    INX_ViewFrustum();

    /** INX_ViewFrustum update */
    void Update(const NX_Camera& camera, float aspect);

    /** Layer culling */
    NX_Layer GetCullMask() const;

    /** Distance to view */
    float GetDistanceSqTo(const NX_Vec3& point) const;
    float GetDistanceSqToCenter(const NX_BoundingBox3D& box, const NX_Transform& transform) const;
    float GetDistanceSqToFarthestCorner(const NX_BoundingBox3D& box, const NX_Transform& transform) const;

    /** Matrices */
    const NX_Vec3& GetViewPosition() const;
    const NX_Mat4& GetViewProj() const;
    const NX_Mat4& GetInvView() const;
    const NX_Mat4& GetInvProj() const;
    const NX_Mat4& GetView() const;
    const NX_Mat4& GetProj() const;

    /** Projection */
    float GetNear() const;
    float GetFar() const;

    /** INX_ViewFrustum UBO */
    const gpu::Buffer& GetBuffer() const;

private:
    struct GPUData {
        alignas(16) NX_Mat4 viewProj{};
        alignas(16) NX_Mat4 view{};
        alignas(16) NX_Mat4 proj{};
        alignas(16) NX_Mat4 invViewProj{};
        alignas(16) NX_Mat4 invView{};
        alignas(16) NX_Mat4 invProj{};
        alignas(16) NX_Vec3 position{};
        alignas(4) uint32_t cullMask{};
        alignas(4) float near;
        alignas(4) float far;
    };

private:
    gpu::Buffer mUniform{};     //< Uniform buffer that stores data
    GPUData mData{};            //< Data uploaded to the uniform buffer
};

/* === Public Implementation === */

inline INX_ViewFrustum::INX_ViewFrustum()
    : mUniform(GL_UNIFORM_BUFFER, sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW)
{ }

inline void INX_ViewFrustum::Update(const NX_Camera& camera, float aspect)
{
    /* --- Save raw data from camera --- */

    mData.position = camera.position;
    mData.cullMask = camera.cullMask;
    mData.near = camera.nearPlane;
    mData.far = camera.farPlane;

    /* --- Compute view matrix --- */

    NX_Mat4 T = NX_Mat4Translate(-camera.position);
    NX_Mat4 R = NX_QuatToMat4(camera.rotation);
    R = NX_Mat4Transpose(&R);

    mData.view = T * R;

    /* --- Compute projection matrix --- */

    switch (camera.projection) {
    case NX_PROJECTION_PERSPECTIVE:
        {
            float top = camera.nearPlane * tanf(camera.fov * 0.5f);
            float right = top * aspect;
            mData.proj = NX_Mat4Frustum(-right, right, -top, top, camera.nearPlane, camera.farPlane);
        }
        break;
    case NX_PROJECTION_ORTHOGRAPHIC:
        {
            float top = camera.fov * 0.5f;
            float right = top * aspect;
            mData.proj = NX_Mat4Ortho(-right, right, -top, top, camera.nearPlane, camera.farPlane);
        }
        break;
    }

    /* --- Compute view/proj matrix --- */

    mData.viewProj = mData.view * mData.proj;

    /* --- Compute inverse matrices --- */

    mData.invViewProj = NX_Mat4Inverse(&mData.viewProj);
    mData.invView = NX_Mat4Inverse(&mData.view);
    mData.invProj = NX_Mat4Inverse(&mData.proj);

    /* --- Compute frustum planes --- */

    INX_Frustum::Update(mData.viewProj);

    /* --- Upload to the uniform buffer --- */

    mUniform.Upload(&mData);
}

inline NX_Layer INX_ViewFrustum::GetCullMask() const
{
    return mData.cullMask;
}

inline float INX_ViewFrustum::GetDistanceSqTo(const NX_Vec3& point) const
{
    return NX_Vec3DistanceSq(mData.position, point);
}

inline float INX_ViewFrustum::GetDistanceSqToCenter(const NX_BoundingBox3D& box, const NX_Transform& transform) const
{
    NX_Vec3 local = (box.min + box.max) * 0.5f;
    NX_Vec3 world = local * transform;

    return NX_Vec3DistanceSq(mData.position, world);
}

inline float INX_ViewFrustum::GetDistanceSqToFarthestCorner(const NX_BoundingBox3D& box, const NX_Transform& transform) const
{
    const NX_Vec3 corners[8] = {
        NX_VEC3(box.min.x, box.min.y, box.min.z) * transform,
        NX_VEC3(box.max.x, box.min.y, box.min.z) * transform,
        NX_VEC3(box.min.x, box.max.y, box.min.z) * transform,
        NX_VEC3(box.max.x, box.max.y, box.min.z) * transform,
        NX_VEC3(box.min.x, box.min.y, box.max.z) * transform,
        NX_VEC3(box.max.x, box.min.y, box.max.z) * transform,
        NX_VEC3(box.min.x, box.max.y, box.max.z) * transform,
        NX_VEC3(box.max.x, box.max.y, box.max.z) * transform
    };

    float maxDistSq = NX_Vec3DistanceSq(mData.position, corners[0]);

    for (int i = 1; i < 8; ++i) {
        float distSq = NX_Vec3DistanceSq(mData.position, corners[i]);
        if (distSq > maxDistSq) maxDistSq = distSq;
    }

    return maxDistSq;
}

inline const NX_Vec3& INX_ViewFrustum::GetViewPosition() const
{
    return mData.position;
}

inline const NX_Mat4& INX_ViewFrustum::GetViewProj() const
{
    return mData.viewProj;
}

inline const NX_Mat4& INX_ViewFrustum::GetInvView() const
{
    return mData.invView;
}

inline const NX_Mat4& INX_ViewFrustum::GetInvProj() const
{
    return mData.invProj;
}

inline const NX_Mat4& INX_ViewFrustum::GetView() const
{
    return mData.view;
}

inline const NX_Mat4& INX_ViewFrustum::GetProj() const
{
    return mData.proj;
}

inline float INX_ViewFrustum::GetNear() const
{
    return mData.near;
}

inline float INX_ViewFrustum::GetFar() const
{
    return mData.far;
}

inline const gpu::Buffer& INX_ViewFrustum::GetBuffer() const
{
    return mUniform;
}

#endif // NX_SCENE_VIEW_FRUSTUM_HPP
