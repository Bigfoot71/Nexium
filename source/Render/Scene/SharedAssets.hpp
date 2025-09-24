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

#ifndef HP_RENDER_SCENE_SHARED_ASSETS_HPP
#define HP_RENDER_SCENE_SHARED_ASSETS_HPP

#include "../../Detail/GPU/Shader.hpp"
#include "../HP_Texture.hpp"
#include "../HP_Font.hpp"

namespace scene {

/* === Declaration === */

class SharedAssets {
public:
    SharedAssets();

    const gpu::Texture& textureBrdfLut() const;
    const gpu::Texture& textureNormal() const;

private:
    gpu::Texture mTextureBrdfLut;
    gpu::Texture mTextureNormal;
};

/* === Public Implementation === */

inline const gpu::Texture& SharedAssets::textureBrdfLut() const
{
    return mTextureBrdfLut;
}

inline const gpu::Texture& SharedAssets::textureNormal() const
{
    return mTextureNormal;
}

} // namespace scene

#endif // HP_RENDER_SCENE_SHARED_ASSETS_HPP
