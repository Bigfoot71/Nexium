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

#ifndef HP_RENDER_OVERLAY_DRAW_CALL_HPP
#define HP_RENDER_OVERLAY_DRAW_CALL_HPP

#include "../../Detail/GPU/Pipeline.hpp"
#include "../HP_Texture.hpp"

namespace overlay {

/* === Declaration === */

struct DrawCall {
    enum Mode {
        SHAPE,
        TEXT
    };

    DrawCall() = default;
    DrawCall(const HP_Texture* t, size_t o);
    DrawCall(const HP_Font* f, size_t o);

    void draw(const gpu::Pipeline& pipeline) const;

    union {
        const HP_Texture* texture;
        const HP_Font* font;
    };

    size_t offset;  // Offset dans le buffer d'indices (en nombre d'indices)
    size_t count;   // Nombre d'indices à dessiner
    Mode mode;
};

/* === Public Implementation === */

inline DrawCall::DrawCall(const HP_Texture* t, size_t o)
    : texture(t), offset(o), count(0), mode(SHAPE)
{ }

inline DrawCall::DrawCall(const HP_Font* f, size_t o)
    : font(f), offset(o), count(0), mode(TEXT)
{ }

inline void DrawCall::draw(const gpu::Pipeline& pipeline) const
{
    SDL_assert(count > 0);
    pipeline.drawElements(GL_TRIANGLES, GL_UNSIGNED_SHORT, offset, count);
}

} // namespace overlay

#endif // HP_RENDER_OVERLAY_DRAW_CALL_HPP
