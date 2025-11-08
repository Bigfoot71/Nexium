/* AssimpHelper.hpp -- Contains a collection of helpers for assimp
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_IMPORT_ASSIMP_HELPER_HPP
#define NX_IMPORT_ASSIMP_HELPER_HPP

#include <NX/NX_Math.h>

#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <assimp/quaternion.h>
#include <assimp/color4.h>
#include <assimp/matrix4x4.h>

namespace import {

/* === Assimp Cast Helpers === */

template<typename Target, typename Source>
Target AssimpCast(const Source& src);

template<>
constexpr NX_Vec2 AssimpCast<NX_Vec2, aiVector2D>(const aiVector2D& src)
{
    return NX_VEC2(src.x, src.y);
}

template<>
constexpr NX_Vec2 AssimpCast<NX_Vec2, aiVector3D>(const aiVector3D& src)
{
    return NX_VEC2(src.x, src.y);
}

template<>
constexpr NX_Vec3 AssimpCast<NX_Vec3, aiVector3D>(const aiVector3D& src)
{
    return NX_VEC3(src.x, src.y, src.z);
}

template<>
constexpr NX_Quat AssimpCast<NX_Quat, aiQuaternion>(const aiQuaternion& src)
{
    return NX_QUAT(src.x, src.y, src.z, src.w);
}

template<>
constexpr NX_Color AssimpCast<NX_Color, aiColor4D>(const aiColor4D& src)
{
    return NX_COLOR(src.r, src.g, src.b, src.a);
}

template<>
constexpr NX_Mat4 AssimpCast<NX_Mat4, aiMatrix4x4>(const aiMatrix4x4& src)
{
    return NX_MAT4_T {
        src.a1, src.b1, src.c1, src.d1,
        src.a2, src.b2, src.c2, src.d2,
        src.a3, src.b3, src.c3, src.d3,
        src.a4, src.b4, src.c4, src.d4
    };
}

} // namespace import

#endif // NX_IMPORT_ASSIMP_HELPER_HPP
