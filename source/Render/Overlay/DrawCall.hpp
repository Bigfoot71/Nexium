/* DrawCall.hpp -- Represents a draw call for the overlay system
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_OVERLAY_DRAW_CALL_HPP
#define NX_RENDER_OVERLAY_DRAW_CALL_HPP

#include "../../Detail/GPU/Pipeline.hpp"
#include "../NX_Texture.hpp"

namespace overlay {

/* === Declaration === */

struct DrawCall {
    enum Mode {
        SHAPE,
        TEXT
    };

    DrawCall() = default;
    DrawCall(const NX_Texture* t, size_t o);
    DrawCall(const NX_Font* f, size_t o);

    void draw(const gpu::Pipeline& pipeline) const;

    union {
        const NX_Texture* texture;
        const NX_Font* font;
    };

    size_t offset;  // Offset in the index buffer (in number of indices)
    size_t count;   // Number of indices to draw
    Mode mode;
};

/* === Public Implementation === */

inline DrawCall::DrawCall(const NX_Texture* t, size_t o)
    : texture(t), offset(o), count(0), mode(SHAPE)
{ }

inline DrawCall::DrawCall(const NX_Font* f, size_t o)
    : font(f), offset(o), count(0), mode(TEXT)
{ }

inline void DrawCall::draw(const gpu::Pipeline& pipeline) const
{
    SDL_assert(count > 0);
    pipeline.drawElements(GL_TRIANGLES, GL_UNSIGNED_SHORT, offset, count);
}

} // namespace overlay

#endif // NX_RENDER_OVERLAY_DRAW_CALL_HPP
