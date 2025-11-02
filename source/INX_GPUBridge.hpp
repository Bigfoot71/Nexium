#ifndef INX_GPU_BRIDGE_HPP
#define INX_GPU_BRIDGE_HPP

#include "./Detail/GPU/Pipeline.hpp"
#include <NX/NX_Render.h>

#include "./INX_GlobalState.hpp"  //< Used to get OpenGL profile (core/es)
#include <SDL3/SDL_video.h>
#include <glad/gles2.h>

// TODO: This header is temporary; the GPU abstraction needs to be reviewed,
//       and translation functions should be omitted.

inline GLenum INX_GPU_GetInternalFormat(NX_PixelFormat format, bool framebuffer)
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

    if (INX_Display.glProfile == SDL_GL_CONTEXT_PROFILE_ES /*!GLAD_GL_EXT_color_buffer_float*/) {
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

inline NX_PixelFormat INX_GPU_GetPixelFormat(GLenum internalFormat)
{
    switch (internalFormat) {
    case GL_R8: return NX_PIXEL_FORMAT_R8;
    case GL_RG8: return NX_PIXEL_FORMAT_RG8;
    case GL_RGB8: return NX_PIXEL_FORMAT_RGB8;
    case GL_RGBA8: return NX_PIXEL_FORMAT_RGBA8;
    case GL_R16F: return NX_PIXEL_FORMAT_R16F;
    case GL_RG16F: return NX_PIXEL_FORMAT_RG16F;
    case GL_RGB16F: return NX_PIXEL_FORMAT_RGB16F;
    case GL_RGBA16F: return NX_PIXEL_FORMAT_RGBA16F;
    case GL_R32F: return NX_PIXEL_FORMAT_R32F;
    case GL_RG32F: return NX_PIXEL_FORMAT_RG32F;
    case GL_RGB32F: return NX_PIXEL_FORMAT_RGB32F;
    case GL_RGBA32F: return NX_PIXEL_FORMAT_RGBA32F;
    default: break;
    }

    return NX_PIXEL_FORMAT_INVALID;
}

inline GLenum INX_GPU_GetPrimitiveType(NX_PrimitiveType type)
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

inline gpu::CullMode INX_GPU_GetCullMode(NX_CullMode mode)
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

inline gpu::CullMode INX_GPU_GetCullMode(NX_ShadowFaceMode shadow, NX_CullMode mode)
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

inline gpu::BlendMode INX_GPU_GetBlendMode(NX_BlendMode mode)
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

inline gpu::DepthFunc INX_GPU_GetDepthFunc(NX_DepthTest func)
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

#endif // INX_GPU_BRIDGE_HPP
