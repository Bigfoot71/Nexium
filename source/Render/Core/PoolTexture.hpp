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

#ifndef HP_RENDER_POOL_TEXTURE_HPP
#define HP_RENDER_POOL_TEXTURE_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Core/HP_InternalLog.hpp"
#include "../HP_Texture.hpp"

namespace render {

/* === Declaration === */

class PoolTexture {
public:
    HP_Texture* create(const HP_Image& image, HP_TextureWrap wrap);
    HP_Texture* create(const HP_Image& image);
    void destroy(HP_Texture* texture);

    void setDefaultFilter(HP_TextureFilter filter);
    void setDefaultAnisotropy(float anisotropy);

private:
    util::ObjectPool<HP_Texture, 1024> mPool;

private:
    HP_TextureFilter mDefaultFilter{HP_TEXTURE_FILTER_BILINEAR};
    float mDefaultAnisotropy{1.0f};
};

/* === Public Implementation === */

inline HP_Texture* PoolTexture::create(const HP_Image& image, HP_TextureWrap wrap)
{
    HP_Texture* texture = mPool.create(image, mDefaultFilter, wrap, mDefaultFilter);
    if (texture == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load texture; Object pool issue");
        return texture;
    }

    if (!texture->isValid()) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load texture; GPU-side issue");
        mPool.destroy(texture);
        return nullptr;
    }

    return texture;
}

inline HP_Texture* PoolTexture::create(const HP_Image& image)
{
    return create(image, HP_TEXTURE_WRAP_CLAMP);
}

inline void PoolTexture::destroy(HP_Texture* texture)
{
    if (texture != nullptr) {
        mPool.destroy(texture);
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
