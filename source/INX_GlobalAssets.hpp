#ifndef INX_GLOBAL_ASSETS_H
#define INX_GLOBAL_ASSETS_H

#include <NX/NX_Texture.h>
#include <NX/NX_Render.h>
#include <array>

// ============================================================================
// ENUMS ASSETS
// ============================================================================

enum class INX_TextureAsset {
    SSAO_KERNEL,
    SSAO_NOISE,
    BRDF_LUT,
    NORMAL,
    WHITE,
    COUNT
};

enum class INX_FontAsset {
    DEFAULT,
    COUNT
};

// ============================================================================
// GLOBAL ASSETS
// ============================================================================

extern class INX_GlobalAssets {
public:
    /** Getters */
    const NX_Texture* Get(INX_TextureAsset asset);
    const NX_Font* Get(INX_FontAsset asset);

    /** Selectors */
    const NX_Texture* Select(const NX_Texture* texture, INX_TextureAsset asset);
    const NX_Font* Select(const NX_Font* font, INX_FontAsset asset);

    /** Unload */
    void Unload();

private:
    std::array<NX_Texture*, int(INX_TextureAsset::COUNT)> mTextures;
    std::array<NX_Font*, int(INX_FontAsset::COUNT)> mFonts;

} INX_Assets;

inline const NX_Texture* INX_GlobalAssets::Select(const NX_Texture* texture, INX_TextureAsset asset)
{
    return texture ? texture : this->Get(asset);
}

inline const NX_Font* INX_GlobalAssets::Select(const NX_Font* font, INX_FontAsset asset)
{
    return font ? font : this->Get(asset);
}

#endif // INX_GLOBAL_ASSETS_H
