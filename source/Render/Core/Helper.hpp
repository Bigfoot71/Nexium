/* Helper.hpp -- Contains a collection of helpers for the renderer
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_HELPER_HPP
#define NX_RENDER_HELPER_HPP

#include "../../Detail/GPU/Pipeline.hpp"
#include "../../Core/NX_CoreState.hpp"

#include <NX/NX_Render.h>
#include <NX/NX_Image.h>
#include <NX/NX_Math.h>
#include <glad/gles2.h>

namespace render {

/* === GL Enum Helpers === */

inline GLenum getInternalFormat(NX_PixelFormat format, bool framebuffer)
{
    GLenum internalFormat = GL_RGBA8;

    switch (format) {
    case NX_PIXEL_FORMAT_R8: internalFormat = GL_R8; break;
    case NX_PIXEL_FORMAT_RG8: internalFormat = GL_RG8; break;
    case NX_PIXEL_FORMAT_RGB8: internalFormat = GL_RGB8; break;
    case NX_PIXEL_FORMAT_RGBA8: internalFormat = GL_RGBA8; break;
    case NX_PIXEL_FORMAT_R16F: internalFormat = GL_R16F; break;
    case NX_PIXEL_FORMAT_RG16F: internalFormat = GL_RG16F; break;
    case NX_PIXEL_FORMAT_RGB16F: internalFormat = GL_RGB16F; break;
    case NX_PIXEL_FORMAT_RGBA16F: internalFormat = GL_RGBA16F; break;
    case NX_PIXEL_FORMAT_R32F: internalFormat = GL_R32F; break;
    case NX_PIXEL_FORMAT_RG32F: internalFormat = GL_RG32F; break;
    case NX_PIXEL_FORMAT_RGB32F: internalFormat = GL_RGB32F; break;
    case NX_PIXEL_FORMAT_RGBA32F: internalFormat = GL_RGBA32F; break;
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
        case NX_PIXEL_FORMAT_R32F: internalFormat = GL_R16F; break;
        case NX_PIXEL_FORMAT_RG32F: internalFormat = GL_RG16F; break;
        case NX_PIXEL_FORMAT_RGB32F: internalFormat = GL_RGB16F; break;
        case NX_PIXEL_FORMAT_RGBA32F: internalFormat = GL_RGBA16F; break;
        default: break;
        }
    }

    return internalFormat;
}

inline GLenum getPrimitiveType(NX_PrimitiveType type)
{
    switch (type) {
    case NX_PRIMITIVE_POINTS:
        return GL_POINTS;
    case NX_PRIMITIVE_LINES:
        return GL_LINES;
    case NX_PRIMITIVE_LINE_STRIP:
        return GL_LINE_STRIP;
    case NX_PRIMITIVE_LINE_LOOP:
        return GL_LINE_LOOP;
    case NX_PRIMITIVE_TRIANGLES:
        return GL_TRIANGLES;
    case NX_PRIMITIVE_TRIANGLE_STRIP:
        return GL_TRIANGLE_STRIP;
    case NX_PRIMITIVE_TRIANGLE_FAN:
        return GL_TRIANGLE_FAN;
    default:
        break;
    }

    return GL_TRIANGLES;
}

inline gpu::CullMode getCullMode(NX_CullMode mode)
{
    switch (mode) {
    case NX_CULL_BACK:
        return gpu::CullMode::Back;
        break;
    case NX_CULL_FRONT:
        return gpu::CullMode::Front;
        break;
    case NX_CULL_NONE:
        return gpu::CullMode::Disabled;
    default:
        break;
    }

    return gpu::CullMode::Back;
}

inline gpu::CullMode getCullMode(NX_ShadowFaceMode shadow, NX_CullMode mode)
{
    switch (shadow) {
    case NX_SHADOW_FACE_AUTO:
        switch (mode) {
        case NX_CULL_BACK:
            return gpu::CullMode::Back;
        case NX_CULL_FRONT:
            return gpu::CullMode::Front;
        case NX_CULL_NONE:
            return gpu::CullMode::Disabled;
    default:
        break;
        }
        break;
    case NX_SHADOW_FACE_FRONT:
        return gpu::CullMode::Back;
    case NX_SHADOW_FACE_BACK:
        return gpu::CullMode::Front;
    case NX_SHADOW_FACE_BOTH:
        return gpu::CullMode::Disabled;
    default:
        break;
    }

    return gpu::CullMode::Back;
}

inline gpu::BlendMode getBlendMode(NX_BlendMode mode)
{
    switch (mode) {
    case NX_BLEND_OPAQUE:
        return gpu::BlendMode::Disabled;
        break;
    case NX_BLEND_ALPHA:
        return gpu::BlendMode::Alpha;
        break;
    case NX_BLEND_ADD:
        return gpu::BlendMode::AddAlpha;
        break;
    case NX_BLEND_MUL:
        return gpu::BlendMode::Multiply;
    default:
        break;
    }

    return gpu::BlendMode::Disabled;
}

inline gpu::DepthFunc getDepthFunc(NX_DepthTest func)
{
    switch (func) {
    case NX_DEPTH_TEST_LESS:
        return gpu::DepthFunc::Less;
        break;
    case NX_DEPTH_TEST_GREATER:
        return gpu::DepthFunc::Greater;
        break;
    case NX_DEPTH_TEST_ALWAYS:
        return gpu::DepthFunc::Always;
    default:
        break;
    }

    return gpu::DepthFunc::Less;
}

/* === Cubemap Helpers === */

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
