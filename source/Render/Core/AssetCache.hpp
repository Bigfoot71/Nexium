/* AssetCache.hpp -- Manage internal asset storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
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
