/* Overlay.cpp -- Overlay system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./Overlay.hpp"

namespace overlay {

Overlay::Overlay(render::ProgramCache& programs, render::AssetCache& assets, HP_AppDesc& desc)
    : mPrograms(programs)
    , mAssets(assets)
{
    /* --- Tweak description --- */

    if (desc.render2D.resolution < HP_IVEC2_ONE) {
        desc.render2D.resolution = HP_GetDisplaySize();
    }

    desc.render2D.sampleCount = HP_MAX(desc.render2D.sampleCount, 1);

    /* --- Create GPU Buffers --- */

    mUniformBuffer = gpu::Buffer(GL_UNIFORM_BUFFER, sizeof(HP_Mat4), nullptr, GL_DYNAMIC_DRAW);

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

    /* --- Upload to vertex buffer --- */

    mVertexBuffer->vbo.upload(0, mVertices.size() * sizeof(HP_Vertex2D), mVertices.data());
    mVertexBuffer->ebo.upload(0, mIndices.size() * sizeof(uint16_t), mIndices.data());

    /* --- Setup pipeline --- */

    gpu::Pipeline pipeline;

    pipeline.setBlendMode(gpu::BlendMode::Premultiplied);
    pipeline.bindVertexArray(mVertexBuffer->vao);
    pipeline.bindUniform(0, mUniformBuffer);
    pipeline.bindFramebuffer(mFramebuffer);
    pipeline.setViewport(mFramebuffer);

    /* --- Render all draw calls --- */

    for (const DrawCall& call : mDrawCalls) {
        switch (call.mode) {
        case DrawCall::SHAPE:
            if (call.texture != nullptr) {
                pipeline.useProgram(mPrograms.overlayTexture());
                pipeline.bindTexture(0, call.texture->gpuTexture());
            }
            else {
                pipeline.useProgram(mPrograms.overlayColor());
            }
            break;
        case DrawCall::TEXT:
            const HP_Font& font = call.font ? *call.font : mAssets.font();
            pipeline.useProgram(font.type() == HP_FONT_SDF ? mPrograms.overlayFontSDF() : mPrograms.overlayFontBitmap());
            pipeline.bindTexture(0, font.gpuTexture());
            break;
        }
        call.draw(pipeline);
    }

    /* --- Rotate vertex buffer --- */

    mVertexBuffer.rotate();

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

    pipeline.useProgram(mPrograms.overlay());

    pipeline.setBlendMode(gpu::BlendMode::Premultiplied);
    pipeline.bindTexture(0, mTargetColor);
    pipeline.draw(GL_TRIANGLES, 3);
}

} // namespace overlay
