/* NX_Texture.hpp -- Implementation of the API for textures
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_TEXTURE_HPP
#define NX_TEXTURE_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Image.h>
#include <utility>

#include "../Detail/GPU/Texture.hpp"
#include "./Core/Helper.hpp"

/* === Declaration === */

class NX_Texture {
public:
    NX_Texture(const NX_Image& image, NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy);
    NX_Texture(int w, int h); // Exists for NX_RenderTexture

    bool isValid() const;

    const gpu::Texture& gpuTexture() const;
    int width() const;
    int height() const;

    void setParameters(NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy);
    void setFilter(NX_TextureFilter filter);
    void setAnisotropy(float anisotropy);
    void setWrap(NX_TextureWrap wrap);
    void generateMipmap();

private:
    static std::pair<GLenum, GLenum> getFilter(NX_TextureFilter filter, bool mipmap);
    static GLenum getWrap(NX_TextureWrap wrap);

private:
    gpu::Texture mTexture;
};

/* === Public Implementation === */

inline NX_Texture::NX_Texture(const NX_Image& image, NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy)
{
    bool genMipmap = (filter == NX_TEXTURE_FILTER_TRILINEAR);

    std::pair<GLenum, GLenum> glFilter = getFilter(filter, genMipmap);
    GLenum glWrap = getWrap(wrap);

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = render::getInternalFormat(image.format, false),
            .data = image.pixels,
            .width = image.w,
            .height = image.h,
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
            .anisotropy = anisotropy
        }
    );
}

inline NX_Texture::NX_Texture(int w, int h)
{
    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGB8,
            .data = nullptr,
            .width = w,
            .height = h,
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );
}

inline bool NX_Texture::isValid() const
{
    return mTexture.isValid();
}

inline const gpu::Texture& NX_Texture::gpuTexture() const
{
    return mTexture;
}

inline int NX_Texture::width() const
{
    return mTexture.width();
}

inline int NX_Texture::height() const
{
    return mTexture.height();
}

inline void NX_Texture::setParameters(NX_TextureFilter filter, NX_TextureWrap wrap, float anisotropy)
{
    std::pair<GLenum, GLenum> glFilter = getFilter(filter, mTexture.hasMipmap());
    GLenum glWrap = getWrap(wrap);

    mTexture.setParameters({
        .minFilter = glFilter.first,
        .magFilter = glFilter.second,
        .sWrap = glWrap,
        .tWrap = glWrap,
        .anisotropy = anisotropy
    });
}

inline void NX_Texture::setFilter(NX_TextureFilter filter)
{
    std::pair<GLenum, GLenum> glFilter = getFilter(filter, mTexture.hasMipmap());
    mTexture.setFilter(glFilter.first, glFilter.second);
}

inline void NX_Texture::setAnisotropy(float anisotropy)
{
    mTexture.setAnisotropy(anisotropy);
}

inline void NX_Texture::setWrap(NX_TextureWrap wrap)
{
    GLenum glWrap = getWrap(wrap);
    mTexture.setWrap(glWrap, glWrap, glWrap);
}

inline void NX_Texture::generateMipmap()
{
    mTexture.generateMipmap();
}

/* === Private Implementation === */

inline std::pair<GLenum, GLenum> NX_Texture::getFilter(NX_TextureFilter filter, bool mipmap)
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

inline GLenum NX_Texture::getWrap(NX_TextureWrap wrap)
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

#endif // NX_TEXTURE_HPP
