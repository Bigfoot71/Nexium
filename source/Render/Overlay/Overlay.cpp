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

#include "./Overlay.hpp"

#include <shaders/generic.vert.h>
#include <shaders/generic.frag.h>
#include <shaders/overlay.frag.h>
#include <assets/font.ttf.h>

namespace overlay {

Overlay::Overlay(const render::SharedAssets& assets, HP_AppDesc& desc)
    : mAssets(assets)
{
    /* --- Tweak description --- */

    if (desc.render2D.resolution < HP_IVEC2_ONE) {
        desc.render2D.resolution = HP_GetDisplaySize();
    }

    desc.render2D.sampleCount = HP_MAX(desc.render2D.sampleCount, 1);

    /* --- Compile shaders --- */

    gpu::Shader vertShader(GL_VERTEX_SHADER, GENERIC_VERT);
    mProgramFontBitmap = gpu::Program(vertShader, gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"FONT_BITMAP"}));
    mProgramFontSDF = gpu::Program(vertShader, gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"FONT_SDF"}));
    mProgramTexture = gpu::Program(vertShader, gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"TEXTURE"}));
    mProgramColor = gpu::Program(vertShader, gpu::Shader(GL_FRAGMENT_SHADER, GENERIC_FRAG, {"COLOR"}));

    mProgramOverlay = gpu::Program(assets.vertexShaderScreen(), gpu::Shader(GL_FRAGMENT_SHADER, OVERLAY_FRAG));

    /* --- Create GPU Buffers --- */

    mUniformBuffer = gpu::Buffer(GL_UNIFORM_BUFFER, sizeof(HP_Mat4), nullptr, GL_DYNAMIC_DRAW);
    mVertexBuffer = gpu::Buffer(GL_ARRAY_BUFFER, MaxVertices * sizeof(HP_Vertex2D), nullptr, GL_DYNAMIC_DRAW);
    mIndexBuffer = gpu::Buffer(GL_ELEMENT_ARRAY_BUFFER, MaxIndices * sizeof(uint16_t), nullptr, GL_DYNAMIC_DRAW);

    mVertexArray = gpu::VertexArray(&mIndexBuffer, {
        gpu::VertexBufferDesc {
            .buffer = &mVertexBuffer,
            .attributes = {
                gpu::VertexAttribute {
                    .location = 0,
                    .size = 2,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(HP_Vertex2D),
                    .offset = offsetof(HP_Vertex2D, position),
                    .divisor = 0
                },
                gpu::VertexAttribute {
                    .location = 1,
                    .size = 2,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(HP_Vertex2D),
                    .offset = offsetof(HP_Vertex2D, texcoord),
                    .divisor = 0
                },
                gpu::VertexAttribute {
                    .location = 2,
                    .size = 4,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(HP_Vertex2D),
                    .offset = offsetof(HP_Vertex2D, color),
                    .divisor = 0
                }
            }
        }
    });

    /* --- Create Framebuffer --- */

    mTargetColor = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = GL_RGBA8,
            .data = nullptr,
            .width = desc.render2D.resolution.x,
            .height = desc.render2D.resolution.y,
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
    );

    mFramebuffer = gpu::Framebuffer({
        &mTargetColor
    });

    if (desc.render2D.sampleCount > 1) {
        mFramebuffer.setSampleCount(desc.render2D.sampleCount);
    }
}

void Overlay::clear()
{
    gpu::Pipeline pipeline;
    pipeline.bindFramebuffer(mFramebuffer);
    pipeline.clear(mFramebuffer, HP_BLANK);
}

void Overlay::flush()
{
    if (mDrawCalls.empty() || mVertices.empty()) {
        return;
    }

    /* --- Upload data --- */

    mVertexBuffer.upload(0, mVertices.size() * sizeof(HP_Vertex2D), mVertices.data());
    mIndexBuffer.upload(0, mIndices.size() * sizeof(uint16_t), mIndices.data());

    /* --- Setup pipeline --- */

    gpu::Pipeline pipeline;

    pipeline.setBlendMode(gpu::BlendMode::Premultiplied);
    pipeline.bindUniform(0, mUniformBuffer);
    pipeline.bindVertexArray(mVertexArray);
    pipeline.bindFramebuffer(mFramebuffer);
    pipeline.setViewport(mFramebuffer);

    /* --- Render all draw calls --- */

    for (const DrawCall& call : mDrawCalls) {
        switch (call.mode) {
        case DrawCall::SHAPE:
            if (call.texture != nullptr) {
                pipeline.useProgram(mProgramTexture);
                pipeline.bindTexture(0, call.texture->gpuTexture());
            }
            else {
                pipeline.useProgram(mProgramColor);
            }
            break;
        case DrawCall::TEXT:
            const HP_Font& font = call.font ? *call.font : mAssets.font();
            pipeline.useProgram(font.type() == HP_FONT_SDF ? mProgramFontSDF : mProgramFontBitmap);
            pipeline.bindTexture(0, font.gpuTexture());
            break;
        }
        call.draw(pipeline);
    }

    /* --- Reset --- */

    mDrawCalls.clear();
    mVertices.clear();
    mIndices.clear();
}

void Overlay::blit()
{
    mFramebuffer.resolve();

    gpu::Pipeline pipeline;

    if (mCurrentTarget != nullptr) {
        pipeline.bindFramebuffer(mCurrentTarget->framebuffer());
        pipeline.setViewport(mCurrentTarget->framebuffer());
    }
    else {
        pipeline.setViewport(HP_GetWindowSize());
    }

    pipeline.useProgram(mProgramOverlay);

    pipeline.setBlendMode(gpu::BlendMode::Premultiplied);
    pipeline.bindTexture(0, mTargetColor);
    pipeline.draw(GL_TRIANGLES, 3);
}

} // namespace overlay
