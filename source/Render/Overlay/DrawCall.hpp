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
#include "../NX_Shader.hpp"

namespace overlay {

/* === Declaration === */

struct DrawCall {
    /** Draw mode */
    enum Mode {
        SHAPE,
        TEXT
    };

    /** Constructors */
    DrawCall() = default;
    DrawCall(NX_Shader* s, const NX_Texture* t, size_t o);
    DrawCall(NX_Shader* s, const NX_Font* f, size_t o);

    /** Helper functions */
    void draw(const gpu::Pipeline& pipeline) const;

    /** Shader related data */
    NX_Shader::TextureArray shaderTextures;
    NX_Shader* shader;
    int uRangeIndex;

    /** Built-in drawable */
    union {
        const NX_Texture* texture;
        const NX_Font* font;
    };

    /** Draw call info */
    size_t offset;          //< Offset in the index buffer (in number of indices)
    size_t count;           //< Number of indices to draw
    Mode mode;
};

/* === Public Implementation === */

inline DrawCall::DrawCall(NX_Shader* s, const NX_Texture* t, size_t o)
    : shader(s), texture(t), offset(o), count(0), mode(SHAPE)
{
    if (s) {
        s->getTextures(shaderTextures);
        uRangeIndex = s->dynamicRangeIndex();
    }
}

inline DrawCall::DrawCall(NX_Shader* s, const NX_Font* f, size_t o)
    : shader(s), font(f), offset(o), count(0), mode(TEXT)
{ }

inline void DrawCall::draw(const gpu::Pipeline& pipeline) const
{
    SDL_assert(count > 0);
    pipeline.drawElements(GL_TRIANGLES, GL_UNSIGNED_SHORT, offset, count);
}

} // namespace overlay

#endif // NX_RENDER_OVERLAY_DRAW_CALL_HPP
