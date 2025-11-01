/* Helper.hpp -- Contains a collection of helpers for the renderer
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_HELPER_HPP
#define NX_RENDER_HELPER_HPP

#include <NX/NX_Math.h>

namespace render {

inline NX_Mat4 getCubeView(int face, const NX_Vec3& eye = NX_VEC3_ZERO)
{
    constexpr NX_Vec3 dirs[6] = {
        {  1.0,  0.0,  0.0 }, // +X
        { -1.0,  0.0,  0.0 }, // -X
        {  0.0,  1.0,  0.0 }, // +Y
        {  0.0, -1.0,  0.0 }, // -Y
        {  0.0,  0.0,  1.0 }, // +Z
        {  0.0,  0.0, -1.0 }  // -Z
    };

    constexpr NX_Vec3 ups[6] = {
        {  0.0, -1.0,  0.0 }, // +X
        {  0.0, -1.0,  0.0 }, // -X
        {  0.0,  0.0,  1.0 }, // +Y
        {  0.0,  0.0, -1.0 }, // -Y
        {  0.0, -1.0,  0.0 }, // +Z
        {  0.0, -1.0,  0.0 }  // -Z
    };

    return NX_Mat4LookAt(eye, eye + dirs[face], ups[face]);
}

inline NX_Mat4 getCubeProj(float near = 0.1f, float far = 10.0f)
{
    return NX_Mat4Perspective(NX_PI / 2.0f, 1.0f, near, far);
}

} // namespace render

#endif // NX_RENDER_HELPER_HPP
