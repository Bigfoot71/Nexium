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

#ifndef HP_RENDER_OVERLAY_HPP
#define HP_RENDER_OVERLAY_HPP

#include "../HP_Font.hpp"
#include "./DrawCall.hpp"

#include "../../Detail/Util/StaticArray.hpp"
#include "../../Detail/GPU/VertexArray.hpp"
#include "../../Detail/GPU/Pipeline.hpp"
#include "../../Detail/GPU/Buffer.hpp"
#include "../Core/SharedAssets.hpp"
#include "../HP_Texture.hpp"

namespace overlay {

class Overlay {
public:
    static constexpr int MaxDrawCalls = 128;
    static constexpr int MaxVertices = 4096;
    static constexpr int MaxIndices = 6144;

public:
    Overlay(const render::SharedAssets& assets, HP_AppDesc& desc);
    ~Overlay() = default;

    Overlay(const Overlay&) = delete;
    Overlay& operator=(const Overlay&) = delete;

    // Gestion de base
    void setProjection(const HP_Mat4& projection);
    void setTexture(const HP_Texture* texture);
    void setFont(const HP_Font* font);
    void setColor(HP_Color color);

    // Ajout de données
    void addVertex(float x, float y, float u, float v);
    void addVertex(const HP_Vertex2D& vertex);
    void addIndex(uint16_t index);

    // Rendu
    void clear();
    void flush();
    void blit();

    // Informations
    uint16_t nextVertexIndex() const;
    const HP_Font& currentFont() const;

    // Signalement de dessin
    void ensureDrawCall(DrawCall::Mode mode, int vertices, int indices);

private:
    /* --- CPU Buffers --- */

    util::StaticArray<DrawCall, MaxDrawCalls> mDrawCalls;
    util::StaticArray<HP_Vertex2D, MaxVertices> mVertices;
    util::StaticArray<uint16_t, MaxIndices> mIndices;

    /* --- GPU Buffers --- */

    gpu::VertexArray mVertexArray{};
    gpu::Buffer mUniformBuffer{};
    gpu::Buffer mVertexBuffer{};
    gpu::Buffer mIndexBuffer{};

    /* --- Program Shaders --- */

    gpu::Program mProgramFontBitmap{};
    gpu::Program mProgramFontSDF{};
    gpu::Program mProgramTexture{};
    gpu::Program mProgramColor{};
    gpu::Program mProgramOverlay{};

    /* --- Framebuffer --- */

    gpu::Framebuffer mFramebuffer{};
    gpu::Texture mTargetColor{};

    /* --- Current State --- */

    HP_Color mCurrentColor = HP_WHITE;
    const HP_Font* mCurrentFont = nullptr;
    const HP_Texture* mCurrentTexture = nullptr;

private:
    const render::SharedAssets& mAssets;
};

/* === Public Implementation === */

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

} // namespace overlay

#endif // HP_RENDER_OVERLAY_HPP
