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

#include "../../Core/HP_CoreState.hpp"

#include <Hyperion/HP_Image.h>
#include <Hyperion/HP_Math.h>
#include <glad/gles2.h>

namespace render {

/* === Format Helpers === */

inline GLenum getInternalFormat(HP_PixelFormat format, bool framebuffer)
{
    GLenum internalFormat = GL_RGBA8;

    switch (format) {
    case HP_PIXEL_FORMAT_R8: internalFormat = GL_R8; break;
    case HP_PIXEL_FORMAT_RG8: internalFormat = GL_RG8; break;
    case HP_PIXEL_FORMAT_RGB8: internalFormat = GL_RGB8; break;
    case HP_PIXEL_FORMAT_RGBA8: internalFormat = GL_RGBA8; break;
    case HP_PIXEL_FORMAT_R16F: internalFormat = GL_R16F; break;
    case HP_PIXEL_FORMAT_RG16F: internalFormat = GL_RG16F; break;
    case HP_PIXEL_FORMAT_RGB16F: internalFormat = GL_RGB16F; break;
    case HP_PIXEL_FORMAT_RGBA16F: internalFormat = GL_RGBA16F; break;
    case HP_PIXEL_FORMAT_R32F: internalFormat = GL_R32F; break;
    case HP_PIXEL_FORMAT_RG32F: internalFormat = GL_RG32F; break;
    case HP_PIXEL_FORMAT_RGB32F: internalFormat = GL_RGB32F; break;
    case HP_PIXEL_FORMAT_RGBA32F: internalFormat = GL_RGBA32F; break;
    default: break;
    }

    if (!framebuffer) {
        return internalFormat;
    }

    // REVIEW: On some emulated GLES 3.2 contexts (e.g. NVIDIA desktop drivers),
    // the extension GL_EXT_color_buffer_float may be reported as supported,
    // but attempting to use 32-bit float color attachments (GL_RGBA32F, etc.)
    // can result in incomplete framebuffers...
    //
    // For maximum compatibility across all GLES 3.2 implementations,
    // we currently force 16-bit float formats (GL_RGBA16F, etc.) for FBO color attachments.
    // This behavior may need to be revisited later.

    if (gCore->glProfile() == SDL_GL_CONTEXT_PROFILE_ES /*&& !GLAD_GL_EXT_color_buffer_float*/) {
        switch (format) {
        case HP_PIXEL_FORMAT_R32F: internalFormat = GL_R16F; break;
        case HP_PIXEL_FORMAT_RG32F: internalFormat = GL_RG16F; break;
        case HP_PIXEL_FORMAT_RGB32F: internalFormat = GL_RGB16F; break;
        case HP_PIXEL_FORMAT_RGBA32F: internalFormat = GL_RGBA16F; break;
        default: break;
        }
    }

    return internalFormat;
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
