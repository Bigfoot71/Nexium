/* PoolTexture.hpp -- Storage pool for textures and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_POOL_TEXTURE_HPP
#define NX_RENDER_POOL_TEXTURE_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Core/NX_InternalLog.hpp"
#include "../NX_RenderTexture.hpp"
#include "../NX_Texture.hpp"

namespace render {

/* === Declaration === */

class PoolTexture {
public:
    /** Texture management */
    NX_Texture* createTexture(const NX_Image& image, NX_TextureWrap wrap);
    NX_Texture* createTexture(const NX_Image& image);
    void destroyTexture(NX_Texture* texture);

    /** Render texture management */
    NX_RenderTexture* createRenderTexture(int w, int h);
    void destroyRenderTexture(NX_RenderTexture* renderTexture);

    /** Default parameters */
    void setDefaultFilter(NX_TextureFilter filter);
    void setDefaultAnisotropy(float anisotropy);

private:
    util::ObjectPool<NX_RenderTexture, 8> mRenderTextures;
    util::ObjectPool<NX_Texture, 1024> mTextures;

private:
    NX_TextureFilter mDefaultFilter{NX_TEXTURE_FILTER_BILINEAR};
    float mDefaultAnisotropy{1.0f};
};

/* === Public Implementation === */

inline NX_Texture* PoolTexture::createTexture(const NX_Image& image, NX_TextureWrap wrap)
{
    NX_Texture* texture = mTextures.create(image, mDefaultFilter, wrap, mDefaultFilter);
    if (texture == nullptr) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to load texture; Object pool issue");
        return texture;
    }

    if (!texture->isValid()) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to load texture; GPU-side issue");
        mTextures.destroy(texture);
        return nullptr;
    }

    return texture;
}

inline NX_Texture* PoolTexture::createTexture(const NX_Image& image)
{
    return createTexture(image, NX_TEXTURE_WRAP_CLAMP);
}

inline void PoolTexture::destroyTexture(NX_Texture* texture)
{
    if (texture != nullptr) {
        mTextures.destroy(texture);
    }
}

inline NX_RenderTexture* PoolTexture::createRenderTexture(int w, int h)
{
    NX_RenderTexture* renderTexture = mRenderTextures.create(w, h);
    if (renderTexture == nullptr) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to create render texture; Object pool issue");
        return renderTexture;
    }

    if (!renderTexture->isValid()) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to create render texture; GPU-side issue");
        mRenderTextures.destroy(renderTexture);
        return nullptr;
    }

    return renderTexture;
}

inline void PoolTexture::destroyRenderTexture(NX_RenderTexture* renderTexture)
{
    if (renderTexture != nullptr) {
        mRenderTextures.destroy(renderTexture);
    }
}

inline void PoolTexture::setDefaultFilter(NX_TextureFilter filter)
{
    mDefaultFilter = filter;
}

inline void PoolTexture::setDefaultAnisotropy(float anisotropy)
{
    mDefaultAnisotropy = anisotropy;
}

} // namespace render

#endif // NX_RENDER_POOL_TEXTURE_HPP
