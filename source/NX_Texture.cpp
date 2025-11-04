/* NX_Texture.cpp -- API definition for Nexium's texture module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Texture.hpp"
#include <NX/NX_Log.h>

#include "./Detail/GPU/Texture.hpp"
#include "./INX_GlobalPool.hpp"
#include "./INX_GPUBridge.hpp"

// ============================================================================
// LOCAL MANAGEMENT
// ============================================================================

static NX_TextureFilter INX_DefaultFilter = NX_TEXTURE_FILTER_BILINEAR;
static NX_TextureWrap INX_DefaultWrap = NX_TEXTURE_WRAP_CLAMP;
static float INX_DefaultAnisotropy = 1.0f;

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

static std::pair<GLenum, GLenum> INX_GetFilter(NX_TextureFilter filter, bool mipmap)
{
    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;

    switch (filter) {
    case NX_TEXTURE_FILTER_POINT:
        minFilter = mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
        magFilter = GL_NEAREST;
        break;
    case NX_TEXTURE_FILTER_BILINEAR:
        minFilter = mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
        magFilter = GL_LINEAR;
        break;
    case NX_TEXTURE_FILTER_TRILINEAR:
        minFilter = mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        magFilter = GL_LINEAR;
        break;
    }

    return std::make_pair(minFilter, magFilter);
}

static GLenum INX_GetWrap(NX_TextureWrap wrap)
{
    GLenum glWrap = GL_CLAMP_TO_EDGE;

    switch (wrap) {
    case NX_TEXTURE_WRAP_CLAMP:
        glWrap = GL_CLAMP_TO_EDGE;
        break;
    case NX_TEXTURE_WRAP_REPEAT:
        glWrap = GL_REPEAT;
        break;
    case NX_TEXTURE_WRAP_MIRROR:
        glWrap = GL_MIRRORED_REPEAT;
        break;
    }

    return glWrap;
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_TextureFilter NX_GetDefaultTextureFilter()
{
    return INX_DefaultFilter;
}

void NX_SetDefaultTextureFilter(NX_TextureFilter filter)
{
    INX_DefaultFilter = filter;
}

float NX_GetDefaultTextureAnisotropy()
{
    return INX_DefaultAnisotropy;
}

void NX_SetDefaultTextureAnisotropy(float anisotropy)
{
    INX_DefaultAnisotropy = anisotropy;
}

NX_Texture* NX_CreateTexture(int w, int h, const void* data, NX_PixelFormat format)
{
    return NX_CreateTextureEx(w, h, data, format, INX_DefaultWrap, INX_DefaultFilter);
}

NX_Texture* NX_CreateTextureEx(int w, int h, const void* data, NX_PixelFormat format, NX_TextureWrap wrap, NX_TextureFilter filter)
{
    if (w <= 0 && h <= 0) {
        NX_LOG(E, "RENDER: Failed to create texture; Dimensions are invalid");
        return nullptr;
    }

    bool genMipmap = (filter == NX_TEXTURE_FILTER_TRILINEAR);
    std::pair<GLenum, GLenum> glFilter = INX_GetFilter(filter, genMipmap);
    GLenum glWrap = INX_GetWrap(wrap);

    gpu::Texture texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            // NOTE: NX_CreateTexture is used to create NX_RenderTexture
            .internalFormat = INX_GPU_GetInternalFormat(format, true),
            .data = data,
            .width = w,
            .height = h,
            .depth = 0,
            .mipmap = genMipmap
        },
        gpu::TextureParam
        {
            .minFilter = glFilter.first,
            .magFilter = glFilter.second,
            .sWrap = glWrap,
            .tWrap = glWrap,
            .rWrap = glWrap,
            .anisotropy = INX_DefaultAnisotropy
        }
    );

    return INX_Pool.Create<NX_Texture>(std::move(texture));
}

NX_Texture* NX_CreateTextureFromImage(const NX_Image* image)
{
    return NX_CreateTextureFromImageEx(image, INX_DefaultWrap, INX_DefaultFilter);
}

NX_Texture* NX_CreateTextureFromImageEx(const NX_Image* image, NX_TextureWrap wrap, NX_TextureFilter filter)
{
    if (image == nullptr) {
        NX_LOG(E, "RENDER: Failed to load texture; Image is null");
        return nullptr;
    }

    bool genMipmap = (filter == NX_TEXTURE_FILTER_TRILINEAR);
    std::pair<GLenum, GLenum> glFilter = INX_GetFilter(filter, genMipmap);
    GLenum glWrap = INX_GetWrap(wrap);

    gpu::Texture texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = INX_GPU_GetInternalFormat(image->format, false),
            .data = image->pixels,
            .width = image->w,
            .height = image->h,
            .depth = 0,
            .mipmap = genMipmap
        },
        gpu::TextureParam
        {
            .minFilter = glFilter.first,
            .magFilter = glFilter.second,
            .sWrap = glWrap,
            .tWrap = glWrap,
            .rWrap = glWrap,
            .anisotropy = INX_DefaultAnisotropy
        }
    );

    return INX_Pool.Create<NX_Texture>(std::move(texture));
}

NX_Texture* NX_LoadTexture(const char* filePath)
{
    NX_Image image = NX_LoadImage(filePath);
    if (image.pixels == nullptr) return nullptr;

    NX_Texture* texture = NX_CreateTextureFromImage(&image);
    NX_DestroyImage(&image);

    return texture;
}

NX_Texture* NX_LoadTextureAsData(const char* filePath)
{
    NX_Image image = NX_LoadImageRaw(filePath);
    NX_Texture* texture = NX_CreateTextureFromImage(&image);
    NX_DestroyImage(&image);
    return texture;
}

void NX_DestroyTexture(NX_Texture* texture)
{
    INX_Pool.Destroy(texture);
}

NX_IVec2 NX_GetTextureSize(const NX_Texture* texture)
{
    return texture->gpu.GetDimensions();
}

void NX_SetTextureParameters(NX_Texture* texture, NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy)
{
    std::pair<GLenum, GLenum> glFilter = INX_GetFilter(filter, texture->gpu.HasMipmap());
    GLenum glWrap = INX_GetWrap(wrap);

    texture->gpu.SetParameters({
        .minFilter = glFilter.first,
        .magFilter = glFilter.second,
        .sWrap = glWrap,
        .tWrap = glWrap,
        .rWrap = glWrap,
        .anisotropy = anisotropy
    });
}

void NX_SetTextureAnisotropy(NX_Texture* texture, float anisotropy)
{
    texture->gpu.SetAnisotropy(anisotropy);
}

void NX_SetTextureFilter(NX_Texture* texture, NX_TextureFilter filter)
{
    std::pair<GLenum, GLenum> glFilter = INX_GetFilter(filter, texture->gpu.HasMipmap());
    texture->gpu.SetFilter(glFilter.first, glFilter.second);
}

void NX_SetTextureWrap(NX_Texture* texture, NX_TextureWrap wrap)
{
    GLenum glWrap = INX_GetWrap(wrap);
    texture->gpu.SetWrap(glWrap, glWrap, glWrap);
}

void NX_UploadTexture(NX_Texture* texture, const NX_Image* image)
{
    NX_Image source = *image;

    NX_PixelFormat texFormat = INX_GPU_GetPixelFormat(texture->gpu.GetInternalFormat());

    if (texFormat != image->format) {
        source = NX_CopyImage(image, texFormat);
    }

    texture->gpu.Upload(image->pixels, 0, 0);
    if (texture->gpu.HasMipmap()) {
        texture->gpu.GenerateMipmap();
    }

    if (source.pixels != image->pixels) {
        NX_DestroyImage(&source);
    }
}

void NX_GenerateMipmap(NX_Texture* texture)
{
    texture->gpu.GenerateMipmap();
}
