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

#ifndef HP_TEXTURE_HPP
#define HP_TEXTURE_HPP

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Image.h>
#include <utility>

#include "../Detail/GPU/Texture.hpp"
#include "./Core/Helper.hpp"

/* === Declaration === */

class HP_Texture {
public:
    HP_Texture(const HP_Image& image, HP_TextureFilter filter, HP_TextureWrap wrap, float anisotropy);
    HP_Texture(int w, int h); // Exists for HP_RenderTexture

    bool isValid() const;

    const gpu::Texture& gpuTexture() const;
    int width() const;
    int height() const;

    void setParameters(HP_TextureFilter filter, HP_TextureWrap wrap, float anisotropy);
    void setFilter(HP_TextureFilter filter);
    void setAnisotropy(float anisotropy);
    void setWrap(HP_TextureWrap wrap);
    void generateMipmap();

private:
    static std::pair<GLenum, GLenum> getFilter(HP_TextureFilter filter, bool mipmap);
    static GLenum getWrap(HP_TextureWrap wrap);

private:
    gpu::Texture mTexture;
};

/* === Public Implementation === */

inline HP_Texture::HP_Texture(const HP_Image& image, HP_TextureFilter filter, HP_TextureWrap wrap, float anisotropy)
{
    bool genMipmap = (filter == HP_TEXTURE_FILTER_TRILINEAR);

    std::pair<GLenum, GLenum> glFilter = getFilter(filter, genMipmap);
    GLenum glWrap = getWrap(wrap);

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = render::getInternalFormat(image.format),
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

inline HP_Texture::HP_Texture(int w, int h)
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

inline bool HP_Texture::isValid() const
{
    return mTexture.isValid();
}

inline const gpu::Texture& HP_Texture::gpuTexture() const
{
    return mTexture;
}

inline int HP_Texture::width() const
{
    return mTexture.width();
}

inline int HP_Texture::height() const
{
    return mTexture.height();
}

inline void HP_Texture::setParameters(HP_TextureFilter filter, HP_TextureWrap wrap, float anisotropy)
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

inline void HP_Texture::setFilter(HP_TextureFilter filter)
{
    std::pair<GLenum, GLenum> glFilter = getFilter(filter, mTexture.hasMipmap());
    mTexture.setFilter(glFilter.first, glFilter.second);
}

inline void HP_Texture::setAnisotropy(float anisotropy)
{
    mTexture.setAnisotropy(anisotropy);
}

inline void HP_Texture::setWrap(HP_TextureWrap wrap)
{
    GLenum glWrap = getWrap(wrap);
    mTexture.setWrap(glWrap, glWrap, glWrap);
}

inline void HP_Texture::generateMipmap()
{
    mTexture.generateMipmap();
}

/* === Private Implementation === */

inline std::pair<GLenum, GLenum> HP_Texture::getFilter(HP_TextureFilter filter, bool mipmap)
{
    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;

    switch (filter) {
    case HP_TEXTURE_FILTER_POINT:
        minFilter = mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
        magFilter = GL_NEAREST;
        break;
    case HP_TEXTURE_FILTER_BILINEAR:
        minFilter = mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
        magFilter = GL_LINEAR;
        break;
    case HP_TEXTURE_FILTER_TRILINEAR:
        minFilter = mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        magFilter = GL_LINEAR;
        break;
    }

    return std::make_pair(minFilter, magFilter);
}

inline GLenum HP_Texture::getWrap(HP_TextureWrap wrap)
{
    GLenum glWrap = GL_CLAMP_TO_EDGE;

    switch (wrap) {
    case HP_TEXTURE_WRAP_CLAMP:
        glWrap = GL_CLAMP_TO_EDGE;
        break;
    case HP_TEXTURE_WRAP_REPEAT:
        glWrap = GL_REPEAT;
        break;
    case HP_TEXTURE_WRAP_MIRROR:
        glWrap = GL_MIRRORED_REPEAT;
        break;
    }

    return glWrap;
}

#endif // HP_TEXTURE_HPP
