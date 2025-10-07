/* AssetCache.hpp -- Manage internal asset storage and on-demand loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_ASSET_CACHE_HPP
#define NX_RENDER_ASSET_CACHE_HPP

#include "../NX_Texture.hpp"
#include "../NX_Font.hpp"

namespace render {

/* === Declaration === */

class AssetCache {
public:
    AssetCache();

    /** Pre-loaded */
    const NX_Texture& textureWhite() const;
    const NX_Font& font() const;

    /** Textures */
    const gpu::Texture& textureSsaoKernel();
    const gpu::Texture& textureSsaoNoise();
    const gpu::Texture& textureBrdfLut();
    const gpu::Texture& textureNormal();
    
    /** Helpers */
    const gpu::Texture& textureOrWhite(const NX_Texture* texture) const;
    const gpu::Texture& textureOrNormal(const NX_Texture* texture);

private:
    /** Pre-loaded */
    NX_Texture mTextureWhite;
    NX_Font mFont;

    /** Textures */
    gpu::Texture mTextureSsaoKernel;
    gpu::Texture mTextureSsaoNoise;
    gpu::Texture mTextureBrdfLut;
    gpu::Texture mTextureNormal;
};

/* === Public Implementation === */

inline const NX_Texture& AssetCache::textureWhite() const
{
    return mTextureWhite;
}

inline const NX_Font& AssetCache::font() const
{
    return mFont;
}

inline const gpu::Texture& AssetCache::textureOrWhite(const NX_Texture* texture) const
{
    return (texture != nullptr) ? texture->gpuTexture() : mTextureWhite.gpuTexture();
}

inline const gpu::Texture& AssetCache::textureOrNormal(const NX_Texture* texture)
{
    // Returns the default by calling 'textureNormal()' to load it if needed
    return (texture != nullptr) ? texture->gpuTexture() : textureNormal();
}

} // namespace render

#endif // NX_RENDER_ASSET_CACHE_HPP
