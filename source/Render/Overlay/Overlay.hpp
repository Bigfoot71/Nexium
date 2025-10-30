/* Overlay.hpp -- Overlay system management class
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_OVERLAY_HPP
#define NX_RENDER_OVERLAY_HPP

#include "../NX_Font.hpp"

#include <NX/NX_Runtime.h>
#include <NX/NX_Display.h>
#include <NX/NX_Window.h>
#include <NX/NX_Init.h>
#include <NX/NX_Math.h>

#include "../../Detail/Util/StaticArray.hpp"
#include "../../Detail/Util/ObjectRing.hpp"
#include "../../Detail/GPU/VertexArray.hpp"
#include "../../Detail/GPU/Pipeline.hpp"
#include "../../Detail/GPU/Buffer.hpp"

#include "../Core/ProgramCache.hpp"
#include "../Core/AssetCache.hpp"

#include "../NX_RenderTexture.hpp"
#include "../NX_Texture.hpp"
#include "./DrawCall.hpp"

namespace overlay {

class Overlay {
public:
    static constexpr int MaxDrawCalls = 128;
    static constexpr int MaxVertices = 4096;
    static constexpr int MaxIndices = 6144;

public:
    Overlay(render::ProgramCache& programs, render::AssetCache& assets, NX_AppDesc& desc);
    ~Overlay() = default;

    Overlay(const Overlay&) = delete;
    Overlay& operator=(const Overlay&) = delete;

    /** Getters */
    const NX_Color& currentColor() const;
    const NX_Font& currentFont() const;
    uint16_t nextVertexIndex() const;

    /** Setters */
    void setRenderTexture(const NX_RenderTexture* target);
    void setProjection(const NX_Mat4& projection);
    void setTexture(const NX_Texture* texture);
    void setFont(const NX_Font* font);
    void setShader(NX_Shader* shader);
    void setColor(NX_Color color);

    /** Transform stack */
    void push();
    void pop();
    void translate(const NX_Vec2& translation);
    void rotate(float radians);
    void scale(const NX_Vec2& scale);

    /** Adding data */
    void addVertex(float x, float y, float u, float v);
    void addVertex(const NX_Vertex2D& vertex);
    void addIndex(uint16_t index);

    /** Render */
    void clear();
    void flush();
    void blit();

    /** Draw call report */
    void ensureDrawCall(DrawCall::Mode mode, int vertices, int indices);

    /** Helpers */
    float toPixelSize(float unit);

private:
    /** GPU vertex buffer */
    struct VertexBuffer {
        gpu::VertexArray vao{};
        gpu::Buffer vbo{};
        gpu::Buffer ebo{};
        VertexBuffer();
    };

    /** GPU uniform data */
    struct Uniform {
        alignas(16) NX_Mat4 projection;
        alignas(4) float time;
    };

private:
    /** CPU Buffers */
    util::StaticArray<DrawCall, MaxDrawCalls> mDrawCalls;
    util::StaticArray<NX_Vertex2D, MaxVertices> mVertices;
    util::StaticArray<uint16_t, MaxIndices> mIndices;

    /** GPU Buffers */
    util::ObjectRing<VertexBuffer, 3> mVertexBuffer{};
    gpu::Buffer mUniformBuffer{};

    /** Framebuffer */
    gpu::Framebuffer mFramebuffer{};
    gpu::Texture mTargetColor{};

    /** Transform stack */
    util::StaticArray<NX_Mat3, 16> mMatrixStack{};

    /** Current State */
    NX_Color mCurrentColor = NX_WHITE;
    NX_Shader* mCurrentShader = nullptr;
    const NX_Font* mCurrentFont = nullptr;
    const NX_Texture* mCurrentTexture = nullptr;
    const NX_RenderTexture* mCurrentTarget = nullptr;

private:
    render::ProgramCache& mPrograms;
    render::AssetCache& mAssets;
};

/* === Public Implementation === */

inline const NX_Color& Overlay::currentColor() const
{
    return mCurrentColor;
}

inline const NX_Font& Overlay::currentFont() const
{
    return mCurrentFont ? *mCurrentFont : mAssets.font();
}

inline uint16_t Overlay::nextVertexIndex() const
{
    return static_cast<uint16_t>(mVertices.size());
}

inline void Overlay::setRenderTexture(const NX_RenderTexture* target)
{
    mCurrentTarget = target;
}

inline void Overlay::setProjection(const NX_Mat4& projection)
{
    // REVIEW: Not very clear upload time here either...

    Uniform data = {
        .projection = projection,
        .time = static_cast<float>(NX_GetElapsedTime())
    };

    mUniformBuffer.upload(&data);
}

inline void Overlay::setTexture(const NX_Texture* texture)
{
    mCurrentTexture = texture;
}

inline void Overlay::setFont(const NX_Font* font)
{
    mCurrentFont = font;
}

inline void Overlay::setShader(NX_Shader* shader)
{
    mCurrentShader = shader;
}

inline void Overlay::setColor(NX_Color color)
{
    mCurrentColor = color;
}

inline void Overlay::push()
{
    if (!mMatrixStack.push_back(*mMatrixStack.back())) {
        NX_LOG(E, "RENDER: Transformation 2D stack overflow");
    }
}

inline void Overlay::pop()
{
    if (mMatrixStack.size() > 1) {
        mMatrixStack.pop_back();
    }
}

inline void Overlay::translate(const NX_Vec2& translation)
{
    NX_Mat3& mat = *mMatrixStack.back();
    mat = mat * NX_Mat3Translate2D(translation);
}

inline void Overlay::rotate(float radians)
{
    NX_Mat3& mat = *mMatrixStack.back();
    mat = mat * NX_Mat3Rotate2D(radians);
}

inline void Overlay::scale(const NX_Vec2& scale)
{
    NX_Mat3& mat = *mMatrixStack.back();
    mat = mat * NX_Mat3Scale2D(scale);
}

inline void Overlay::addVertex(float x, float y, float u, float v)
{
    SDL_assert(mVertices.size() < MaxVertices);
    mVertices.emplace_back(
        NX_VEC2(x, y) * (*mMatrixStack.back()),
        NX_VEC2(u, v), mCurrentColor
    );
}

inline void Overlay::addVertex(const NX_Vertex2D& vertex)
{
    SDL_assert(mVertices.size() < MaxVertices);
    mVertices.emplace_back(
        vertex.position * (*mMatrixStack.back()),
        vertex.texcoord, vertex.color
    );
}

inline void Overlay::addIndex(uint16_t index)
{
    SDL_assert(mIndices.size() < MaxIndices);
    mIndices.emplace_back(index);
    mDrawCalls.back()->count++;
}

inline void Overlay::ensureDrawCall(DrawCall::Mode mode, int vertices, int indices)
{
    if (mVertices.size() + vertices > MaxVertices || mIndices.size() + indices > MaxIndices) {
        flush();
    }

    if (mDrawCalls.empty()) {
        switch (mode) {
        case DrawCall::SHAPE:
            mDrawCalls.emplace_back(mCurrentShader, mCurrentTexture, 0);
            break;
        case DrawCall::TEXT:
            mDrawCalls.emplace_back(mCurrentShader, mCurrentFont, 0);
            break;
        }
        return;
    }

    DrawCall& call = *mDrawCalls.back();

    if (call.count == 0) {
        call.shader = mCurrentShader;
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

    if (call.mode == mode && call.shader == mCurrentShader) {
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
        mDrawCalls.emplace_back(mCurrentShader, mCurrentTexture, mIndices.size());
        break;
    case DrawCall::TEXT:
        mDrawCalls.emplace_back(mCurrentShader, mCurrentFont, mIndices.size());
        break;
    }
}

inline float Overlay::toPixelSize(float unit)
{
    if (!NX_IsMat3Identity(mMatrixStack.back())) {
        const NX_Mat3& mat = *mMatrixStack.back();
        float scaleX = sqrtf(mat.m00 * mat.m00 + mat.m01 * mat.m01);
        float scaleY = sqrtf(mat.m10 * mat.m10 + mat.m11 * mat.m11);
        float avgScale = (scaleX + scaleY) * 0.5f;
        unit /= avgScale;
    }
    return unit;
}

/* === Overlay::VertexBuffer - Implementation === */

inline Overlay::VertexBuffer::VertexBuffer()
{
    vbo = gpu::Buffer(GL_ARRAY_BUFFER, MaxVertices * sizeof(NX_Vertex2D), nullptr, GL_DYNAMIC_DRAW);
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
                    .stride = sizeof(NX_Vertex2D),
                    .offset = offsetof(NX_Vertex2D, position),
                    .divisor = 0
                },
                gpu::VertexAttribute {
                    .location = 1,
                    .size = 2,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(NX_Vertex2D),
                    .offset = offsetof(NX_Vertex2D, texcoord),
                    .divisor = 0
                },
                gpu::VertexAttribute {
                    .location = 2,
                    .size = 4,
                    .type = GL_FLOAT,
                    .normalized = false,
                    .stride = sizeof(NX_Vertex2D),
                    .offset = offsetof(NX_Vertex2D, color),
                    .divisor = 0
                }
            }
        }
    });
}

} // namespace overlay

#endif // NX_RENDER_OVERLAY_HPP
