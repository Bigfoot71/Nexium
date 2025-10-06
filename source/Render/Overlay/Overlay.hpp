/* Overlay.hpp -- Overlay system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_RENDER_OVERLAY_HPP
#define HP_RENDER_OVERLAY_HPP

#include "../HP_Font.hpp"
#include "./DrawCall.hpp"

#include "../../Detail/Util/StaticArray.hpp"
#include "../../Detail/Util/ObjectRing.hpp"
#include "../../Detail/GPU/VertexArray.hpp"
#include "../../Detail/GPU/Pipeline.hpp"
#include "../../Detail/GPU/Buffer.hpp"

#include "../Core/ProgramCache.hpp"
#include "../Core/AssetCache.hpp"

#include "../HP_RenderTexture.hpp"
#include "../HP_Texture.hpp"

namespace overlay {

class Overlay {
public:
    static constexpr int MaxDrawCalls = 128;
    static constexpr int MaxVertices = 4096;
    static constexpr int MaxIndices = 6144;

public:
    Overlay(render::ProgramCache& programs, render::AssetCache& assets, HP_AppDesc& desc);
    ~Overlay() = default;

    Overlay(const Overlay&) = delete;
    Overlay& operator=(const Overlay&) = delete;

    /** Setters */
    void setRenderTexture(const HP_RenderTexture* target);
    void setProjection(const HP_Mat4& projection);
    void setTexture(const HP_Texture* texture);
    void setFont(const HP_Font* font);
    void setColor(HP_Color color);

    /** Adding data */
    void addVertex(float x, float y, float u, float v);
    void addVertex(const HP_Vertex2D& vertex);
    void addIndex(uint16_t index);

    /** Render */
    void clear();
    void flush();
    void blit();

    /** Infos */
    uint16_t nextVertexIndex() const;
    const HP_Font& currentFont() const;

    /** Draw call report */
    void ensureDrawCall(DrawCall::Mode mode, int vertices, int indices);

private:
    /** GPU Vertex buffer */
    struct VertexBuffer {
        gpu::VertexArray vao{};
        gpu::Buffer vbo{};
        gpu::Buffer ebo{};
        VertexBuffer();
    };

private:
    /** CPU Buffers */
    util::StaticArray<DrawCall, MaxDrawCalls> mDrawCalls;
    util::StaticArray<HP_Vertex2D, MaxVertices> mVertices;
    util::StaticArray<uint16_t, MaxIndices> mIndices;

    /** GPU Buffers */
    util::ObjectRing<VertexBuffer, 3> mVertexBuffer{};
    gpu::Buffer mUniformBuffer{};

    /** Framebuffer */
    gpu::Framebuffer mFramebuffer{};
    gpu::Texture mTargetColor{};

    /** Current State */
    HP_Color mCurrentColor = HP_WHITE;
    const HP_Font* mCurrentFont = nullptr;
    const HP_Texture* mCurrentTexture = nullptr;
    const HP_RenderTexture* mCurrentTarget = nullptr;

private:
    render::ProgramCache& mPrograms;
    render::AssetCache& mAssets;
};

/* === Public Implementation === */

inline void Overlay::setRenderTexture(const HP_RenderTexture* target)
{
    mCurrentTarget = target;
}

inline void Overlay::setProjection(const HP_Mat4& projection)
{
    mUniformBuffer.upload(&projection);
}

inline void Overlay::setTexture(const HP_Texture* texture)
{
    mCurrentTexture = texture;
}

inline void Overlay::setFont(const HP_Font* font)
{
    mCurrentFont = font;
}

inline void Overlay::setColor(HP_Color color)
{
    mCurrentColor = color;
}

inline void Overlay::addVertex(float x, float y, float u, float v)
{
    SDL_assert(mVertices.size() < MaxVertices);
    mVertices.emplace_back(HP_VEC2(x, y), HP_VEC2(u, v), mCurrentColor);
}

inline void Overlay::addVertex(const HP_Vertex2D& vertex)
{
    SDL_assert(mVertices.size() < MaxVertices);
    mVertices.emplace_back(vertex);
}

inline void Overlay::addIndex(uint16_t index)
{
    SDL_assert(mIndices.size() < MaxIndices);
    mIndices.emplace_back(index);
    mDrawCalls.back()->count++;
}

inline uint16_t Overlay::nextVertexIndex() const
{
    return static_cast<uint16_t>(mVertices.size());
}

inline const HP_Font& Overlay::currentFont() const
{
    return mCurrentFont ? *mCurrentFont : mAssets.font();
}

inline void Overlay::ensureDrawCall(DrawCall::Mode mode, int vertices, int indices)
{
    if (mVertices.size() + vertices > MaxVertices || mIndices.size() + indices > MaxIndices) {
        flush();
    }

    if (mDrawCalls.empty()) {
        switch (mode) {
        case DrawCall::SHAPE:
            mDrawCalls.emplace_back(mCurrentTexture, 0);
            break;
        case DrawCall::TEXT:
            mDrawCalls.emplace_back(mCurrentFont, 0);
            break;
        }
        return;
    }

    DrawCall& call = *mDrawCalls.back();

    if (call.count == 0) {
        call.mode = mode;
        switch (mode) {
        case DrawCall::SHAPE:
            call.texture = mCurrentTexture;
            break;
        case DrawCall::TEXT:
            call.font = mCurrentFont;
            break;
        }
        return;
    }

    if (call.mode == mode) {
        switch (call.mode) {
        case DrawCall::SHAPE:
            if (call.texture == mCurrentTexture) {
                return;
            }
            break;
        case DrawCall::TEXT:
            if (call.font == mCurrentFont) {
                return;
            }
            break;
        }
    }

    if (mDrawCalls.size() == MaxDrawCalls) {
        flush();
    }

    switch (mode) {
    case DrawCall::SHAPE:
        mDrawCalls.emplace_back(mCurrentTexture, mIndices.size());
        break;
    case DrawCall::TEXT:
        mDrawCalls.emplace_back(mCurrentFont, mIndices.size());
        break;
    }
}

/* === Overlay::VertexBuffer - Implementation === */

inline Overlay::VertexBuffer::VertexBuffer()
{
    vbo = gpu::Buffer(GL_ARRAY_BUFFER, MaxVertices * sizeof(HP_Vertex2D), nullptr, GL_DYNAMIC_DRAW);
    ebo = gpu::Buffer(GL_ELEMENT_ARRAY_BUFFER, MaxIndices * sizeof(uint16_t), nullptr, GL_DYNAMIC_DRAW);

    vao = gpu::VertexArray(&ebo, {
        gpu::VertexBufferDesc {
            .buffer = &vbo,
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
}

} // namespace overlay

#endif // HP_RENDER_OVERLAY_HPP
