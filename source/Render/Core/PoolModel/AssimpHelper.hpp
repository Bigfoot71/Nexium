/* AssimpHelper.hpp -- Contains a collection of helpers for assimp
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_RENDER_POOL_MODEL_ASSIMP_HELPER_HPP
#define HP_RENDER_POOL_MODEL_ASSIMP_HELPER_HPP

#include <Hyperion/HP_Math.h>

#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <assimp/quaternion.h>
#include <assimp/color4.h>
#include <assimp/matrix4x4.h>

/* === Assimp Cast Helpers === */

template<typename Target, typename Source>
Target assimp_cast(const Source& src);

template<>
constexpr HP_Vec2 assimp_cast<HP_Vec2, aiVector2D>(const aiVector2D& src)
{
    return HP_VEC2(src.x, src.y);
}

template<>
constexpr HP_Vec2 assimp_cast<HP_Vec2, aiVector3D>(const aiVector3D& src)
{
    return HP_VEC2(src.x, src.y);
}

template<>
constexpr HP_Vec3 assimp_cast<HP_Vec3, aiVector3D>(const aiVector3D& src)
{
    return HP_VEC3(src.x, src.y, src.z);
}

template<>
constexpr HP_Quat assimp_cast<HP_Quat, aiQuaternion>(const aiQuaternion& src)
{
    return HP_QUAT(src.w, src.x, src.y, src.z);
}

template<>
constexpr HP_Color assimp_cast<HP_Color, aiColor4D>(const aiColor4D& src)
{
    return HP_COLOR(src.r, src.g, src.b, src.a);
}

template<>
constexpr HP_Mat4 assimp_cast<HP_Mat4, aiMatrix4x4>(const aiMatrix4x4& src)
{
    return HP_MAT4_T {
        src.a1, src.b1, src.c1, src.d1,
        src.a2, src.b2, src.c2, src.d2,
        src.a3, src.b3, src.c3, src.d3,
        src.a4, src.b4, src.c4, src.d4
    };
}

#endif // HP_RENDER_POOL_MODEL_ASSIMP_HELPER_HPP
