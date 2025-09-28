/* PoolTexture.hpp -- Storage pool for textures and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_RENDER_POOL_TEXTURE_HPP
#define HP_RENDER_POOL_TEXTURE_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Core/HP_InternalLog.hpp"
#include "../HP_RenderTexture.hpp"
#include "../HP_Texture.hpp"

namespace render {

/* === Declaration === */

class PoolTexture {
public:
    HP_Texture* createTexture(const HP_Image& image, HP_TextureWrap wrap);
    HP_Texture* createTexture(const HP_Image& image);
    void destroyTexture(HP_Texture* texture);

    HP_RenderTexture* createRenderTexture(int w, int h);
    void destroyRenderTexture(HP_RenderTexture* renderTexture);

    void setDefaultFilter(HP_TextureFilter filter);
    void setDefaultAnisotropy(float anisotropy);

private:
    util::ObjectPool<HP_RenderTexture, 8> mRenderTextures;
    util::ObjectPool<HP_Texture, 1024> mTextures;

private:
    HP_TextureFilter mDefaultFilter{HP_TEXTURE_FILTER_BILINEAR};
    float mDefaultAnisotropy{1.0f};
};

/* === Public Implementation === */

inline HP_Texture* PoolTexture::createTexture(const HP_Image& image, HP_TextureWrap wrap)
{
    HP_Texture* texture = mTextures.create(image, mDefaultFilter, wrap, mDefaultFilter);
    if (texture == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load texture; Object pool issue");
        return texture;
    }

    if (!texture->isValid()) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load texture; GPU-side issue");
        mTextures.destroy(texture);
        return nullptr;
    }

    return texture;
}

inline HP_Texture* PoolTexture::createTexture(const HP_Image& image)
{
    return createTexture(image, HP_TEXTURE_WRAP_CLAMP);
}

inline void PoolTexture::destroyTexture(HP_Texture* texture)
{
    if (texture != nullptr) {
        mTextures.destroy(texture);
    }
}

inline HP_RenderTexture* PoolTexture::createRenderTexture(int w, int h)
{
    HP_RenderTexture* renderTexture = mRenderTextures.create(w, h);
    if (renderTexture == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to create render texture; Object pool issue");
        return renderTexture;
    }

    if (!renderTexture->isValid()) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to create render texture; GPU-side issue");
        mRenderTextures.destroy(renderTexture);
        return nullptr;
    }

    return renderTexture;
}

inline void PoolTexture::destroyRenderTexture(HP_RenderTexture* renderTexture)
{
    if (renderTexture != nullptr) {
        mRenderTextures.destroy(renderTexture);
    }
}

inline void PoolTexture::setDefaultFilter(HP_TextureFilter filter)
{
    mDefaultFilter = filter;
}

inline void PoolTexture::setDefaultAnisotropy(float anisotropy)
{
    mDefaultAnisotropy = anisotropy;
}

} // namespace render

#endif // HP_RENDER_POOL_TEXTURE_HPP
