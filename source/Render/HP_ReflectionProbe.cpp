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

#include "./HP_ReflectionProbe.hpp"

/* === Public Implementation === */

HP_ReflectionProbe::HP_ReflectionProbe(const HP_Cubemap& cubemap, gpu::Program& programIrradiance, gpu::Program& programPrefilter)
    : mIrradiance(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = GL_RGB16F,
            .data = nullptr,
            .width = 32,
            .height = 32,
            .depth = 0,
            .mipmap = false
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    )
    , mPrefilter(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = GL_RGB16F,
            .data = nullptr,
            .width = 128,
            .height = 128,
            .depth = 0,
            .mipmap = true
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR_MIPMAP_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    )
    , mFBIrradiance({&mIrradiance})
    , mFBPrefilter({&mPrefilter})
{
    genIrradiance(cubemap, programIrradiance);
    genPrefilter(cubemap, programPrefilter);
}

/* === Private Implementation === */

void HP_ReflectionProbe::genIrradiance(const HP_Cubemap& cubemap, gpu::Program& programIrradiance)
{
    gpu::Pipeline pipeline;

    pipeline.bindFramebuffer(mFBIrradiance);
    pipeline.setViewport(mFBIrradiance);

    pipeline.bindTexture(0, cubemap.texture());
    pipeline.useProgram(programIrradiance);

    for (int i = 0; i < 6; i++) {
        mFBIrradiance.setColorAttachmentTarget(0, 0, i);
        pipeline.setUniformMat4(0, render::getCubeView(i) * render::getCubeProj());
        pipeline.draw(GL_TRIANGLES, 36);
    }
}

void HP_ReflectionProbe::genPrefilter(const HP_Cubemap& cubemap, gpu::Program& programPrefilter)
{
    gpu::Pipeline pipeline;

    pipeline.bindFramebuffer(mFBPrefilter);
    pipeline.setViewport(mFBPrefilter);

    pipeline.bindTexture(0, cubemap.texture());
    pipeline.useProgram(programPrefilter);

    pipeline.setUniformFloat1(1, cubemap.dimensions().x);
    pipeline.setUniformInt1(2, cubemap.numLevels());

    int baseSize = mFBPrefilter.width();
    for (int mip = 0; mip < mPrefilter.numLevels(); mip++) {
        int mipSize = std::max(1, baseSize >> mip);
        pipeline.setViewport(0, 0, mipSize, mipSize);
        pipeline.setUniformFloat1(3, static_cast<float>(mip) / (mPrefilter.numLevels() - 1));
        for (int i = 0; i < 6; i++) {
            mFBPrefilter.setColorAttachmentTarget(0, 0, i, mip);
            pipeline.setUniformMat4(0, render::getCubeView(i) * render::getCubeProj());
            pipeline.draw(GL_TRIANGLES, 36);
        }
    }
}
