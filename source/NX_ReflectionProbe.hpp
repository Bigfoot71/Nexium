/* NX_ReflectionProbe.hpp -- API definition for Nexium's reflection probe module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_REFLECTION_PROBE_HPP
#define NX_REFLECTION_PROBE_HPP

#include <NX/NX_ReflectionProbe.h>

#include "./Detail/GPU/Framebuffer.hpp"
#include "./Detail/GPU/Texture.hpp"

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

struct NX_ReflectionProbe {
    gpu::Texture irradiance;
    gpu::Texture prefilter;
};

#endif // NX_REFLECTION_PROBE_HPP
