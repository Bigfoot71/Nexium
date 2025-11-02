/* NX_Cubemap.hpp -- API definition for Nexium's cubemap module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CUBEMAP_HPP
#define NX_CUBEMAP_HPP

#include <NX/NX_Cubemap.h>

#include "./Detail/GPU/Framebuffer.hpp"
#include "./Detail/GPU/Texture.hpp"

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

struct NX_Cubemap {
    gpu::Texture gpu;
    gpu::Framebuffer framebuffer;  //< Invalid by default, created only if needed
};

#endif // NX_CUBEMAP_HPP
