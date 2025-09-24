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

#ifndef HP_RENDER_POOL_FONT_HPP
#define HP_RENDER_POOL_FONT_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Core/HP_InternalLog.hpp"
#include "../HP_Font.hpp"

namespace render {

/* === Declaration === */

class PoolFont {
public:
    HP_Font* create(const void* fileData, size_t dataSize, HP_FontType type, int baseSize, int* codepoints, int codepointCount);
    void destroy(HP_Font* font);

private:
    util::ObjectPool<HP_Font, 32> mPool{};
};

/* === Public Implementation === */

inline HP_Font* PoolFont::create(const void* fileData, size_t dataSize, HP_FontType type, int baseSize, int* codepoints, int codepointCount)
{
    HP_Font* font = mPool.create(fileData, dataSize, type, baseSize, codepoints, codepointCount);
    if (font == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load font; Object pool issue");
    }
    if (!font->isValid()) {
        mPool.destroy(font);
        return nullptr;
    }
    return font;
}

inline void PoolFont::destroy(HP_Font* font)
{
    if (font != nullptr) {
        mPool.destroy(font);
    }
}

} // namespace render

#endif // HP_RENDER_POOL_FONT_HPP
