/* NX_Render3D.hpp -- API definition for Nexium's 3D renderer module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_3D_HPP
#define NX_RENDER_3D_HPP

#include <NX/NX_Init.h>
#include <NX/NX_Light.h>

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

/** Should be called in NX_Init() */
bool INX_Render3DState_Init(NX_AppDesc* desc);

/** Should be call in NX_Quit() */
void INX_Render3DState_Quit();

/** Should be call by NX_Light to get a shadow map */
int INX_Render3DState_RequestShadowMap(NX_LightType type);

/** Should be call by NX_Light to release a shadow map */
void INX_Render3DState_ReleaseShadowMap(NX_LightType type, int mapIndex);

#endif // NX_RENDER_3D_HPP
