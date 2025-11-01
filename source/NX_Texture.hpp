/* NX_Texture.hpp -- API definitions for Nexium's texture module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_TEXTURE_HPP
#define NX_TEXTURE_HPP

#include <NX/NX_Texture.h>
#include "./Detail/GPU/Texture.hpp"

struct NX_Texture {
    gpu::Texture gpu;
};

#endif // NX_TEXTURE_HPP
