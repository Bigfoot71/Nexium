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

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

/** Should be called in NX_Init() */
bool INX_Render3DState_Init(NX_AppDesc* desc);

/** Should be call in NX_Quit() */
void INX_Render3DState_Quit();

#endif // NX_RENDER_3D_HPP
