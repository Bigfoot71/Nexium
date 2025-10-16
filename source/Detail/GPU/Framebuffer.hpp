/* Framebuffer.hpp -- Allows high-level GPU framebuffer management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_FRAMEBUFFER_HPP
#define NX_GPU_FRAMEBUFFER_HPP

#include "../../Core/NX_InternalLog.hpp"
#include "../Util/FixedArray.hpp"
#include "./TextureView.hpp"
#include "../BuildInfo.hpp"
#include "./Texture.hpp"

#include <NX/NX_Core.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>
#include <glad/gles2.h>

#include <initializer_list>
#include <utility>

namespace gpu {

/* === Declaration === */

/**
 * Framebuffer allows the creation of a FBO with a set of color attachments and an
 * optional depth/stencil attachment. It automatically manages multisample internally
 * by creating and maintaining the required multisampled renderbuffers. 
 *
 * Resolution of multisampled renderbuffers into the attached textures is not automatic 
 * and must be performed explicitly via the provided resolve method.
 *
 * Warning points:
 *  - Attachments (color and depth/stencil) are mostly immutable after construction.
 *  - The framebuffer stores TextureViews of the attached textures. These TextureViews
 *    capture the state of the textures at the time of attachment.
 *  - It is strictly forbidden to modify the storage of any attached texture after
 *    framebuffer creation (width, height, target), as it can break the internal
 *    logic for multisampling and renderbuffer management, and can also invalidate
 *    internal safety/debug checks.
 *  - Exception, updating the depth aspect (and so the mip level count) of the attached
 *    textures is allowed via `updateColorTextureView` (for color attachments) and 
 *    `updateDepthTextureView` (for depth/stencil attachments). Only changes to
 *    depth is safe, other modifications remain forbidden.
 */
class Framebuffer {
public:
    /** Constructors */
    Framebuffer() = default;
    Framebuffer(std::initializer_list<const Texture*> colorAttachments, const Texture* depthStencilAttachment = nullptr) noexcept;

    /** Destructor and Move semantics */
    ~Framebuffer() noexcept;
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    /** Public interface */
    bool isValid() const noexcept;
    GLuint resolveId() const noexcept;  // Always returns the resolve framebuffer (with original, sampleable textures)
    GLuint renderId() const noexcept;   // Always returns the active framebuffer (MSAA or normal)
    int width() const noexcept;
    int height() const noexcept;
    NX_IVec2 dimensions() const noexcept;

    /** Attachment access (always the original textures) */
    const TextureView& getColorAttachment(int index) const noexcept;
    const TextureView& getDepthAttachment() const noexcept;
    size_t colorAttachmentCount() const noexcept;

    /** Draw buffers control */
    void setDrawBuffers(std::initializer_list<int> buffers) noexcept;
    void enableDrawBuffers() noexcept;
    void disableDrawBuffers() noexcept;

    /** Invalidate content */
    void invalidate(std::initializer_list<int> buffers) noexcept;
    void invalidate() noexcept;

    /** Multisampling control */
    void setSampleCount(int sampleCount) noexcept;
    int getSampleCount() const noexcept;

    /** Resolve (blit multisampled renderbuffers to original textures) */
    void resolve() noexcept;

    /** Layered rendering support */
    void setColorAttachmentTarget(int attachmentIndex, int layer = 0, int face = 0, int level = 0) noexcept;
    void setDepthAttachmentTarget(int layer = 0, int face = 0, int level = 0) noexcept;

    /** @warning These methods only accept depth and mip level changes */
    void updateColorTextureView(int attachmentIndex, const gpu::Texture& texture) noexcept;
    void updateDepthTextureView(const gpu::Texture& texture) noexcept;

    /** Get current layer/face targets */
    int getColorAttachmentLayer(int attachmentIndex) const noexcept;
    int getColorAttachmentFace(int attachmentIndex) const noexcept;
    int getColorAttachmentLevel(int attachmentIndex) const noexcept;
    int getDepthAttachmentLayer() const noexcept;
    int getDepthAttachmentFace() const noexcept;
    int getDepthAttachmentLevel() const noexcept;

private:
    /** Structure to track layer/face targets for each attachment */
    struct AttachmentTarget {
        int layer{0};
        int face{0};
        int level{0};
    };

    /** Member variables */
    GLuint mResolveFramebuffer{0};                      //< Framebuffer with original textures
    GLuint mMultisampleFramebuffer{0};                  //< Framebuffer MSAA (optional)
    util::FixedArray<TextureView> mColorAttachments;
    util::FixedArray<GLuint> mColorRenderbuffers;       //< MSAA color renderbuffers
    GLuint mDepthStencilRenderbuffer{0};                //< MSAA depth/stencil renderbuffer
    TextureView mDepthStencilAttachment{};
    int mSampleCount{0};

    /** Layer/face tracking */
    util::FixedArray<AttachmentTarget> mColorTargets;
    AttachmentTarget mDepthTarget;

    /** Utility functions */
    void createResolveFramebuffer() noexcept;
    void createMultisampleFramebuffer() noexcept;
    void destroyMultisampleFramebuffer() noexcept;
    void attachTexturesToResolveFramebuffer() noexcept;
    void createAndAttachMultisampleRenderbuffers() noexcept;
    bool checkFramebufferComplete(GLuint framebuffer) noexcept;
    void updateColorAttachment(int index, bool bind) noexcept;
    void updateDepthAttachment(bool bind) noexcept;
    void resolveColorAttachment(int index) noexcept;
    void resolveDepthAttachment() noexcept;

    /** Static helpers */
    static GLenum getDepthStencilAttachment(GLenum internalFormat) noexcept;
};

/* === Public Implementation === */

inline Framebuffer::Framebuffer(std::initializer_list<const Texture*> colorAttachments, const Texture* depthStencilAttachment) noexcept
{
    if (colorAttachments.size() == 0) {
        NX_INTERNAL_LOG(E, "GPU: Framebuffer requires at least one color attachment");
        return;
    }

    /* --- Validate texture attachments (debug only) --- */

    if constexpr (detail::BuildInfo::debug) {
        NX_IVec2 expectedDims{};
        bool first = true;
        for (const Texture* tex : colorAttachments)  {
            SDL_assert(tex != nullptr);
            if (first) expectedDims = tex->dimensions(), first = false;
            else SDL_assert(tex->dimensions() == expectedDims);
        }
        if (depthStencilAttachment) {
            SDL_assert(depthStencilAttachment->dimensions() == expectedDims);
        }
    }

    /* --- Push all color attachments --- */

    if (!mColorAttachments.reset(colorAttachments.size())) {
        NX_INTERNAL_LOG(E, "GPU: Failed to allocate space to store color attachments IDs");
        return;
    }

    for (size_t i = 0; i < colorAttachments.size(); ++i) {
        mColorAttachments.emplace_back(*colorAttachments.begin()[i]);
    }

    /* --- Create color targets array --- */

    mColorTargets.reset(colorAttachments.size());
    mColorTargets.resize(colorAttachments.size());

    /* --- Validate depth/stencil attachment if provided and keep its ID --- */

    if (depthStencilAttachment) {
        if (!depthStencilAttachment->isValid()) {
            NX_INTERNAL_LOG(E, "GPU: Invalid depth/stencil attachment");
            return;
        }
        mDepthStencilAttachment = *depthStencilAttachment;
    }

    createResolveFramebuffer();

    if (isValid()) {
        enableDrawBuffers();
    }
}

inline Framebuffer::~Framebuffer() noexcept
{
    destroyMultisampleFramebuffer();
    if (mResolveFramebuffer != 0) {
        glDeleteFramebuffers(1, &mResolveFramebuffer);
    }
}

inline Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : mResolveFramebuffer(std::exchange(other.mResolveFramebuffer, 0))
    , mMultisampleFramebuffer(std::exchange(other.mMultisampleFramebuffer, 0))
    , mColorAttachments(std::move(other.mColorAttachments))
    , mColorRenderbuffers(std::move(other.mColorRenderbuffers))
    , mDepthStencilRenderbuffer(std::exchange(other.mDepthStencilRenderbuffer, 0))
    , mDepthStencilAttachment(other.mDepthStencilAttachment)
    , mSampleCount(other.mSampleCount)
    , mColorTargets(std::move(other.mColorTargets))
    , mDepthTarget(other.mDepthTarget)
{ }

inline Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        destroyMultisampleFramebuffer();
        if (mResolveFramebuffer != 0) {
            glDeleteFramebuffers(1, &mResolveFramebuffer);
        }
        mResolveFramebuffer = std::exchange(other.mResolveFramebuffer, 0);
        mMultisampleFramebuffer = std::exchange(other.mMultisampleFramebuffer, 0);
        mColorAttachments = std::move(other.mColorAttachments);
        mColorRenderbuffers = std::move(other.mColorRenderbuffers);
        mDepthStencilRenderbuffer = std::exchange(other.mDepthStencilRenderbuffer, 0);
        mDepthStencilAttachment = other.mDepthStencilAttachment;
        mSampleCount = other.mSampleCount;
        mColorTargets = std::move(other.mColorTargets);
        mDepthTarget = other.mDepthTarget;
    }
    return *this;
}

inline bool Framebuffer::isValid() const noexcept
{
    return (mResolveFramebuffer > 0);
}

inline GLuint Framebuffer::resolveId() const noexcept
{
    // Always returns the resolve framebuffer (with original textures)
    return mResolveFramebuffer;
}

inline GLuint Framebuffer::renderId() const noexcept
{
    // Returns the active framebuffer for rendering (MSAA if enabled, otherwise the resolve framebuffer)
    return (mSampleCount > 0 && mMultisampleFramebuffer > 0) ? mMultisampleFramebuffer : mResolveFramebuffer;
}

inline int Framebuffer::width() const noexcept
{
    return mColorAttachments[0].width();
}

inline int Framebuffer::height() const noexcept
{
    return mColorAttachments[0].height();
}

inline NX_IVec2 Framebuffer::dimensions() const noexcept
{
    return mColorAttachments[0].dimensions();
}

inline const TextureView& Framebuffer::getColorAttachment(int index) const noexcept
{
    return mColorAttachments[index];
}

inline const TextureView& Framebuffer::getDepthAttachment() const noexcept
{
    return mDepthStencilAttachment;
}

inline size_t Framebuffer::colorAttachmentCount() const noexcept
{
    return mColorAttachments.size();
}

inline void Framebuffer::setSampleCount(int sampleCount) noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot set sample count on invalid framebuffer");
        return;
    }

    if (sampleCount < 0) {
        NX_INTERNAL_LOG(E, "GPU: Sample count cannot be negative");
        return;
    }

    if (sampleCount == mSampleCount) {
        return; // No change needed
    }

    mSampleCount = sampleCount;

    if (sampleCount == 0) {
        // No MSAA, renderbuffers remain but are not used
        return;
    }

    // Create or recreate the MSAA framebuffer
    createMultisampleFramebuffer();
}

inline int Framebuffer::getSampleCount() const noexcept
{
    return mSampleCount;
}

inline void Framebuffer::setColorAttachmentTarget(int attachmentIndex, int layer, int face, int level) noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot set attachment target on invalid framebuffer");
        return;
    }

    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorAttachments.size()));
    const TextureView& texture = mColorAttachments[attachmentIndex];

    // Validate mipmap level
    SDL_assert(level >= 0 && level < texture.numLevels());

    // Validate target
    GLenum target = texture.target();

    // Validate layer and face parameters
    if (target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(layer >= 0 && layer < texture.depth());
        if (target == GL_TEXTURE_2D_ARRAY) {
            SDL_assert(face == 0);
        }
    }

    // Check cubemap target
    if (target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(face >= 0 && face < 6);
    }

    mColorTargets[attachmentIndex].layer = layer;
    mColorTargets[attachmentIndex].face = face;
    mColorTargets[attachmentIndex].level = level;

    updateColorAttachment(attachmentIndex, true);
}

inline void Framebuffer::setDepthAttachmentTarget(int layer, int face, int level) noexcept
{
    if (!isValid() || !mDepthStencilAttachment.isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot set depth attachment target on invalid framebuffer or no depth attachment");
        return;
    }

    // Validate mipmap level
    SDL_assert(level >= 0 && level < mDepthStencilAttachment.numLevels());

    // Validate target
    GLenum target = mDepthStencilAttachment.target();
    SDL_assert(target != GL_TEXTURE_2D);

    // Validate layer and face parameters
    if (target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(layer >= 0 && layer < mDepthStencilAttachment.depth());
        if (target == GL_TEXTURE_2D_ARRAY) {
            SDL_assert(face == 0);
        }
    }

    // Check cubemap target
    if (target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(face >= 0 && face < 6);
    }

    mDepthTarget.layer = layer;
    mDepthTarget.face = face;
    mDepthTarget.level = level;

    updateDepthAttachment(true);
}

inline void Framebuffer::updateColorTextureView(int attachmentIndex, const gpu::Texture& texture) noexcept
{
    // Only depth and mip count changes are accepted

    SDL_assert(texture.id() == mColorAttachments[attachmentIndex].id());
    SDL_assert(texture.target() == mColorAttachments[attachmentIndex].target());
    SDL_assert(texture.dimensions() == mColorAttachments[attachmentIndex].dimensions());
    SDL_assert(texture.internalFormat() == mColorAttachments[attachmentIndex].internalFormat());

    mColorAttachments[attachmentIndex] = gpu::TextureView(texture);
}

inline void Framebuffer::updateDepthTextureView(const gpu::Texture& texture) noexcept
{
    // Only depth and mip count changes are accepted

    SDL_assert(texture.id() == mDepthStencilAttachment.id());
    SDL_assert(texture.target() == mDepthStencilAttachment.target());
    SDL_assert(texture.dimensions() == mDepthStencilAttachment.dimensions());
    SDL_assert(texture.internalFormat() == mDepthStencilAttachment.internalFormat());

    mDepthStencilAttachment = gpu::TextureView(texture);
}

inline int Framebuffer::getColorAttachmentLayer(int attachmentIndex) const noexcept
{
    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorTargets.size()));
    return mColorTargets[attachmentIndex].layer;
}

inline int Framebuffer::getColorAttachmentFace(int attachmentIndex) const noexcept
{
    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorTargets.size()));
    return mColorTargets[attachmentIndex].face;
}

inline int Framebuffer::getColorAttachmentLevel(int attachmentIndex) const noexcept
{
    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorTargets.size()));
    return mColorTargets[attachmentIndex].level;
}

inline int Framebuffer::getDepthAttachmentLayer() const noexcept
{
    return mDepthTarget.layer;
}

inline int Framebuffer::getDepthAttachmentFace() const noexcept
{
    return mDepthTarget.face;
}

inline int Framebuffer::getDepthAttachmentLevel() const noexcept
{
    return mDepthTarget.level;
}

/* === Private Implementation === */

inline void Framebuffer::createResolveFramebuffer() noexcept
{
    glGenFramebuffers(1, &mResolveFramebuffer);
    if (mResolveFramebuffer == 0) {
        NX_INTERNAL_LOG(E, "GPU: Failed to create resolve framebuffer object");
        return;
    }

    attachTexturesToResolveFramebuffer();

    if (!checkFramebufferComplete(mResolveFramebuffer)) {
        glDeleteFramebuffers(1, &mResolveFramebuffer);
        mResolveFramebuffer = 0;
        return;
    }
}

inline void Framebuffer::createMultisampleFramebuffer() noexcept
{
    if (mSampleCount <= 0) {
        return;
    }

    // Create multisampled framebuffer if needed
    if (mMultisampleFramebuffer == 0) {
        glGenFramebuffers(1, &mMultisampleFramebuffer);
        if (mMultisampleFramebuffer == 0) {
            NX_INTERNAL_LOG(E, "GPU: Failed to create multisampled framebuffer");
            mSampleCount = 0;
            return;
        }
    }

    createAndAttachMultisampleRenderbuffers();

    if (!checkFramebufferComplete(mMultisampleFramebuffer)) {
        NX_INTERNAL_LOG(E, "GPU: Multisampled framebuffer is not complete");
        destroyMultisampleFramebuffer();
        mSampleCount = 0;
    }

    enableDrawBuffers();
}

inline void Framebuffer::destroyMultisampleFramebuffer() noexcept
{
    if (!mColorRenderbuffers.empty()) {
        glDeleteRenderbuffers(static_cast<GLsizei>(mColorRenderbuffers.size()), mColorRenderbuffers.data());
        mColorRenderbuffers.clear();
    }
    if (mDepthStencilRenderbuffer > 0) {
        glDeleteRenderbuffers(1, &mDepthStencilRenderbuffer);
        mDepthStencilRenderbuffer = 0;
    }
    if (mMultisampleFramebuffer > 0) {
        glDeleteFramebuffers(1, &mMultisampleFramebuffer);
        mMultisampleFramebuffer = 0;
    }
}

/* === Private Static Implementation === */

inline GLenum Framebuffer::getDepthStencilAttachment(GLenum internalFormat) noexcept
{
    switch (internalFormat) {
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32F:
        return GL_DEPTH_ATTACHMENT;
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return GL_DEPTH_STENCIL_ATTACHMENT;
    default:
        NX_INTERNAL_LOG(W, "GPU: Unknown depth/stencil format, using GL_DEPTH_ATTACHMENT");
        return GL_DEPTH_ATTACHMENT;
    }
}

} // namespace gpu

#endif // NX_GPU_FRAMEBUFFER_HPP
