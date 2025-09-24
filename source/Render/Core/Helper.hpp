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

#ifndef HP_RENDER_HELPER_HPP
#define HP_RENDER_HELPER_HPP

#include <Hyperion/HP_Image.h>
#include <Hyperion/HP_Math.h>
#include <glad/gles2.h>

namespace render {

/* === Format Helpers === */

inline GLenum getInternalFormat(HP_PixelFormat format)
{
    switch (format) {
    case HP_PIXEL_FORMAT_R8:        return GL_R8;
    case HP_PIXEL_FORMAT_RG8:       return GL_RG8;
    case HP_PIXEL_FORMAT_RGB8:      return GL_RGB8;
    case HP_PIXEL_FORMAT_RGBA8:     return GL_RGBA8;
    case HP_PIXEL_FORMAT_R16F:      return GL_R16F;
    case HP_PIXEL_FORMAT_RG16F:     return GL_RG16F;
    case HP_PIXEL_FORMAT_RGB16F:    return GL_RGB16F;
    case HP_PIXEL_FORMAT_RGBA16F:   return GL_RGBA16F;
    case HP_PIXEL_FORMAT_R32F:      return GL_R32F;
    case HP_PIXEL_FORMAT_RG32F:     return GL_RG32F;
    case HP_PIXEL_FORMAT_RGB32F:    return GL_RGB32F;
    case HP_PIXEL_FORMAT_RGBA32F:   return GL_RGBA32F;
    default: break;
    }

    return GL_RGBA8;
}

/* === Cubemap Helpers === */

inline HP_Mat4 getCubeView(int face, const HP_Vec3& eye = HP_VEC3_ZERO)
{
    constexpr HP_Vec3 dirs[6] = {
        {  1.0,  0.0,  0.0 }, // +X
        { -1.0,  0.0,  0.0 }, // -X
        {  0.0,  1.0,  0.0 }, // +Y
        {  0.0, -1.0,  0.0 }, // -Y
        {  0.0,  0.0,  1.0 }, // +Z
        {  0.0,  0.0, -1.0 }  // -Z
    };

    constexpr HP_Vec3 ups[6] = {
        {  0.0, -1.0,  0.0 }, // +X
        {  0.0, -1.0,  0.0 }, // -X
        {  0.0,  0.0,  1.0 }, // +Y
        {  0.0,  0.0, -1.0 }, // -Y
        {  0.0, -1.0,  0.0 }, // +Z
        {  0.0, -1.0,  0.0 }  // -Z
    };

    return HP_Mat4LookAt(eye, eye + dirs[face], ups[face]);
}

inline HP_Mat4 getCubeProj(float near = 0.25f, float far = 2.5f)
{
    return HP_Mat4Perspective(HP_PI / 2.0f, 1.0f, near, far);
}

} // namespace render

#endif // HP_RENDER_HELPER_HPP
