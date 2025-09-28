/* TextureView.hpp -- Read-only GPU texture state view
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_GPU_TEXTURE_VIEW_HPP
#define HP_GPU_TEXTURE_VIEW_HPP

#include "./Texture.hpp"

namespace gpu {

/* === Declaration === */

/**
 * TextureView captures the state of a Texture when it is constructed.
 * It does NOT own the underlying texture resource and therefore will
 * not delete or modify it. This view is immutable, any subsequent
 * changes to the original Texture (such as reallocations, resizing,
 * or mipmap changes) are NOT tracked by this view.
 *
 * This class is intended for read-only access to texture properties
 * such as ID, target, internal format, dimensions, and mip levels.
 * Use this when a stable snapshot of a texture is needed, e.g.
 * for framebuffer attachments, without transferring ownership.
 */
class TextureView {
public:
    TextureView() noexcept = default;
    TextureView(const Texture& texture) noexcept;

    bool isValid() const noexcept;

    GLuint id() const noexcept;
    GLenum target() const noexcept;
    GLenum internalFormat() const noexcept;
    HP_IVec2 dimensions() const noexcept;
    int width() const noexcept;
    int height() const noexcept;
    int depth() const noexcept;
    int numLevels() const noexcept;

private:
    GLuint mID{0};
    GLenum mTarget{GL_TEXTURE_2D};
    GLenum mInternalFormat{GL_RGBA8};
    int mWidth{0}, mHeight{0}, mDepth{0};
    int mMipLevels{1};
};

/* === Public Implementation === */

inline TextureView::TextureView(const Texture& texture) noexcept
    : mID(texture.id())
    , mTarget(texture.target())
    , mInternalFormat(texture.internalFormat())
    , mWidth(texture.width())
    , mHeight(texture.height())
    , mDepth(texture.depth())
    , mMipLevels(texture.numLevels())
{ }

inline bool TextureView::isValid() const noexcept
{
    return (mID > 0);
}

inline GLenum TextureView::id() const noexcept
{
    return mID;
}

inline GLenum TextureView::target() const noexcept
{
    return mTarget;
}

inline GLenum TextureView::internalFormat() const noexcept
{
    return mInternalFormat;
}

inline HP_IVec2 TextureView::dimensions() const noexcept
{
    return HP_IVEC2(mWidth, mHeight);
}

inline int TextureView::width() const noexcept
{
    return mWidth;
}

inline int TextureView::height() const noexcept
{
    return mHeight;
}

inline int TextureView::depth() const noexcept
{
    return mDepth;
}

inline int TextureView::numLevels() const noexcept
{
    return mMipLevels;
}

} // namespace gpu

#endif // HP_GPU_TEXTURE_VIEW_HPP
