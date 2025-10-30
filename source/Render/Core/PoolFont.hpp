/* PoolFont.hpp -- Storage pool for fonts and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_POOL_FONT_HPP
#define NX_RENDER_POOL_FONT_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include <NX/NX_Log.h>

#include "../NX_Font.hpp"

namespace render {

/* === Declaration === */

class PoolFont {
public:
    NX_Font* create(const void* fileData, size_t dataSize, NX_FontType type, int baseSize, int* codepoints, int codepointCount);
    void destroy(NX_Font* font);

private:
    util::ObjectPool<NX_Font, 32> mPool{};
};

/* === Public Implementation === */

inline NX_Font* PoolFont::create(const void* fileData, size_t dataSize, NX_FontType type, int baseSize, int* codepoints, int codepointCount)
{
    NX_Font* font = mPool.create(fileData, dataSize, type, baseSize, codepoints, codepointCount);
    if (font == nullptr) {
        NX_LOG(E, "RENDER: Failed to load font; Object pool issue");
    }
    if (!font->isValid()) {
        mPool.destroy(font);
        return nullptr;
    }
    return font;
}

inline void PoolFont::destroy(NX_Font* font)
{
    if (font != nullptr) {
        mPool.destroy(font);
    }
}

} // namespace render

#endif // NX_RENDER_POOL_FONT_HPP
