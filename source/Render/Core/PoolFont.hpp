/* PoolFont.hpp -- Storage pool for fonts and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
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
