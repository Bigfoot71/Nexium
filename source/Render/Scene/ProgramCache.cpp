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

#include "./ProgramCache.hpp"

#include <shaders/light_culling.comp.h>
#include <shaders/forward.vert.h>
#include <shaders/forward.frag.h>
#include <shaders/skybox.vert.h>
#include <shaders/skybox.frag.h>
#include <shaders/shadow.vert.h>
#include <shaders/shadow.frag.h>
#include <shaders/output.frag.h>

namespace scene {

/* === Public Implementation === */

ProgramCache::ProgramCache(const gpu::Shader& vertScreen)
    : mVertexShaderScreen(vertScreen)
{
    mForward = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, FORWARD_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, FORWARD_FRAG)
    );

    mLightCulling = gpu::Program(
        gpu::Shader(GL_COMPUTE_SHADER, LIGHT_CULLING_COMP)
    );

    mShadow = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, SHADOW_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, SHADOW_FRAG)
    );
}

gpu::Program& ProgramCache::skybox()
{
    if (!mSkybox.isValid()) {
        mSkybox = gpu::Program(
            gpu::Shader(
                GL_VERTEX_SHADER,
                SKYBOX_VERT
            ),
            gpu::Shader(
                GL_FRAGMENT_SHADER,
                SKYBOX_FRAG
            )
        );
    }
    return mSkybox;
}

/* --- Private Implementation === */

void ProgramCache::buildOutput(HP_Tonemap tonemap)
{
    const char* tonemapper = "TONEMAPPER TONEMAP_LINEAR";

    switch (tonemap) {
    case HP_TONEMAP_LINEAR:
        break;
    case HP_TONEMAP_REINHARD:
        tonemapper = "TONEMAPPER TONEMAP_REINHARD";
        break;
    case HP_TONEMAP_FILMIC:
        tonemapper = "TONEMAPPER TONEMAP_FILMIC";
        break;
    case HP_TONEMAP_ACES:
        tonemapper = "TONEMAPPER TONEMAP_ACES";
        break;
    case HP_TONEMAP_AGX:
        tonemapper = "TONEMAPPER TONEMAP_AGX";
        break;
    default:
        HP_INTERNAL_LOG(W, "RENDER: Unknown tonemap mode (%i); Linear will be used", tonemap);
        break;
    }

    gpu::Shader frag(GL_FRAGMENT_SHADER, OUTPUT_FRAG, {tonemapper});
    mOutput[tonemap] = gpu::Program(mVertexShaderScreen, frag);
}

} // namespace scene
