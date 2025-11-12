/* NX_Render3D.hpp -- API definition for Nexium's 3D renderer module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_3D_HPP
#define NX_RENDER_3D_HPP

#include <NX/NX_Light.h>
#include <NX/NX_Init.h>

#include "./Detail/GPU/Texture.hpp"

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

/** Should be called in NX_Init() */
bool INX_Render3DState_Init(NX_AppDesc* desc);

/** Should be called in NX_Quit() */
void INX_Render3DState_Quit();

/** Should be called by NX_Light to get a shadow map */
int INX_Render3DState_RequestShadowMap(NX_LightType type);

/** Should be called by NX_Light to release a shadow map */
void INX_Render3DState_ReleaseShadowMap(NX_LightType type, int mapIndex);

/** Should be called by NX_ReflectionProbe to get a probe cubemap */
int INX_Render3DState_RequestProbe();

/** Should be called by NX_ReflectionProbe to release a probe cubemap */
void INX_Render3DState_ReleaseProbe(int probeIndex);

/** Should be called by NX_ReflectionProbe to irradiance cubemaps */
const gpu::Texture& INX_Render3DState_GetIrradianceArray();

/** Should be called by NX_ReflectionProbe to prefilter cubemaps */
const gpu::Texture& INX_Render3DState_GetPrefilterArray();

/** Should be called */

#endif // NX_RENDER_3D_HPP
