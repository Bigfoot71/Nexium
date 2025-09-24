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

#include "./PoolCubemap.hpp"

#include <shaders/cubemap_from_equirectangular.frag.h>
#include <shaders/cubemap_irradiance.frag.h>
#include <shaders/cubemap_prefilter.frag.h>

namespace render {

PoolCubemap::PoolCubemap(const gpu::Shader& vertScreen, const gpu::Shader& cubeScreen)
    : mProgramEquirectangular(vertScreen, gpu::Shader(GL_FRAGMENT_SHADER, CUBEMAP_FROM_EQUIRECTANGULAR_FRAG))
    , mProgramIrradiance(cubeScreen, gpu::Shader(GL_FRAGMENT_SHADER, CUBEMAP_IRRADIANCE_FRAG))
    , mProgramPrefilter(cubeScreen, gpu::Shader(GL_FRAGMENT_SHADER, CUBEMAP_PREFILTER_FRAG))
{ }

} // namespace render
