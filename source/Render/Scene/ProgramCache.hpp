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

#ifndef HP_SCENE_PROGRAM_CACHE_HPP
#define HP_SCENE_PROGRAM_CACHE_HPP

#include <Hyperion/HP_Render.h>

#include "../../Detail/GPU/Program.hpp"
#include "../../Detail/GPU/Shader.hpp"
#include "../Core/SharedAssets.hpp"

namespace scene {

/* === Declaration === */

class ProgramCache {
public:
    ProgramCache(const gpu::Shader& vertScreen);

    gpu::Program& lightCulling();
    gpu::Program& forward();
    gpu::Program& skybox();
    gpu::Program& shadow();

    gpu::Program& output(HP_Tonemap tonemap);

private:
    void buildOutput(HP_Tonemap tonemap);

private:
    std::array<gpu::Program, HP_TONEMAP_COUNT> mOutput{};
    gpu::Program mLightCulling{};
    gpu::Program mForward{};
    gpu::Program mSkybox{};
    gpu::Program mShadow{};

private:
    const gpu::Shader& mVertexShaderScreen;
};

/* === Public Implementation === */

inline gpu::Program& ProgramCache::lightCulling()
{
    return mLightCulling;
}

inline gpu::Program& ProgramCache::forward()
{
    return mForward;
}

inline gpu::Program& ProgramCache::shadow()
{
    return mShadow;
}

inline gpu::Program& ProgramCache::output(HP_Tonemap tonemap)
{
    if (!mOutput[tonemap].isValid()) {
        buildOutput(tonemap);
    }
    return mOutput[tonemap];
}

} // namespace scene

#endif // HP_SCENE_PROGRAM_CACHE_HPP
