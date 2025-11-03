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

#include "./Scene/Scene.hpp"

#include <cfloat>

/* === Global State === */

extern util::UniquePtr<class NX_RenderState> gRender;

/* === Declaration === */

class NX_RenderState {
public:
    /** Renderers */
    scene::Scene scene;

public:
    /** Constructors */
    NX_RenderState(NX_AppDesc& desc);
    ~NX_RenderState() = default;
};

/* === Public Implementation === */

inline NX_RenderState::NX_RenderState(NX_AppDesc& desc)
    : scene(desc)
{ }

#endif // NX_RENDER_STATE_HP
