/* HP_RenderState.hpp -- Contains and manages the global renderer state of Hyperion
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_RENDER_STATE_HP
#define HP_RENDER_STATE_HP

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Init.h>

#include "./Core/ProgramCache.hpp"
#include "./Core/AssetCache.hpp"

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
    render::AssetCache assets;
    render::ProgramCache programs;
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
    , programs()
    , textures()
    , cubemaps(programs)
    , meshes()
    , fonts()
    , models(textures, meshes)
    , overlay(programs, assets, desc)
    , scene(programs, assets, desc)
{ }

#endif // HP_RENDER_STATE_HP
