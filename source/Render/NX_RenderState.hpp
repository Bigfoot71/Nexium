/* NX_RenderState.hpp -- Contains and manages the global renderer state of Nexium
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_STATE_HP
#define NX_RENDER_STATE_HP

#include <NX/NX_Render.h>
#include <NX/NX_Init.h>

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

extern std::unique_ptr<class NX_RenderState> gRender;

/* === Declaration === */

class NX_RenderState {
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
    NX_RenderState(NX_AppDesc& desc);
    ~NX_RenderState() = default;
};

/* === Public Implementation === */

inline NX_RenderState::NX_RenderState(NX_AppDesc& desc)
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

#endif // NX_RENDER_STATE_HP
