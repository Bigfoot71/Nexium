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

#ifndef HP_RENDER_ASSET_CACHE_HPP
#define HP_RENDER_ASSET_CACHE_HPP

#include "../HP_Texture.hpp"
#include "../HP_Font.hpp"

namespace render {

/* === Declaration === */

class AssetCache {
public:
    AssetCache();

    /** Pre-loaded */
    const HP_Texture& textureWhite() const;
    const HP_Font& font() const;

    /** Textures */
    const gpu::Texture& textureSsaoKernel();
    const gpu::Texture& textureSsaoNoise();
    const gpu::Texture& textureBrdfLut();
    const gpu::Texture& textureNormal();
    
    /** Helpers */
    const gpu::Texture& textureOrWhite(const HP_Texture* texture) const;
    const gpu::Texture& textureOrNormal(const HP_Texture* texture);

private:
    /** Pre-loaded */
    HP_Texture mTextureWhite;
    HP_Font mFont;

    /** Textures */
    gpu::Texture mTextureSsaoKernel;
    gpu::Texture mTextureSsaoNoise;
    gpu::Texture mTextureBrdfLut;
    gpu::Texture mTextureNormal;
};

/* === Public Implementation === */

inline const HP_Texture& AssetCache::textureWhite() const
{
    return mTextureWhite;
}

inline const HP_Font& AssetCache::font() const
{
    return mFont;
}

inline const gpu::Texture& AssetCache::textureOrWhite(const HP_Texture* texture) const
{
    return (texture != nullptr) ? texture->gpuTexture() : mTextureWhite.gpuTexture();
}

inline const gpu::Texture& AssetCache::textureOrNormal(const HP_Texture* texture)
{
    // Returns the default by calling 'textureNormal()' to load it if needed
    return (texture != nullptr) ? texture->gpuTexture() : textureNormal();
}

} // namespace render

#endif // HP_RENDER_ASSET_CACHE_HPP
