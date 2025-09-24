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

#ifndef HP_RENDER_STATE_HP
#define HP_RENDER_STATE_HP

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Init.h>

#include "./Core/SharedAssets.hpp"
#include "./Core/PoolTexture.hpp"
#include "./Core/PoolCubemap.hpp"
#include "./Core/PoolModel.hpp"
#include "./Core/PoolMesh.hpp"
#include "./Core/PoolFont.hpp"

#include "./Overlay/Overlay.hpp"
#include "./Scene/Scene.hpp"

#include <memory>
#include <cfloat>

/* === Global State === */

extern std::unique_ptr<class HP_RenderState> gRender;

/* === Declaration === */

class HP_RenderState {
public:
    /** Resource Managers */
    render::SharedAssets assets;
    render::PoolTexture textures;
    render::PoolCubemap cubemaps;
    render::PoolMesh meshes;
    render::PoolFont fonts;
    render::PoolModel models;

    /** Renderers */
    overlay::Overlay overlay;
    scene::Scene scene;

public:
    /** Constructors */
    HP_RenderState(HP_AppDesc& desc);
    ~HP_RenderState() = default;
};

/* === Public Implementation === */

inline HP_RenderState::HP_RenderState(HP_AppDesc& desc)
    : assets() //< Shared assets must be loaded first
    , textures()
    , cubemaps(assets.vertexShaderScreen(), assets.vertexShaderCube())
    , meshes()
    , fonts()
    , models(textures, meshes)
    , overlay(assets, desc)
    , scene(assets, desc)
{ }

#endif // HP_RENDER_STATE_HP
