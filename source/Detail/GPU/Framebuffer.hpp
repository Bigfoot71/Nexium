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

#ifndef HP_GPU_FRAMEBUFFER_HPP
#define HP_GPU_FRAMEBUFFER_HPP

#include "../../Core/HP_InternalLog.hpp"
#include "../Util/DynamicArray.hpp"
#include "./Texture.hpp"

#include <Hyperion/HP_Core.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>
#include <glad/gles2.h>

#include <initializer_list>
#include <utility>

namespace gpu {

/* === Declaration === */

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
    HP_IVec2 dimensions() const noexcept;

    /** Attachment access (always the original textures) */
    const Texture* getColorAttachment(int index) const noexcept;
    const Texture* getDepthAttachment() const noexcept;
    size_t colorAttachmentCount() const noexcept;

    /** Draw buffers control */
    void setDrawBuffers(std::initializer_list<GLenum> buffers) noexcept;
    void enableAllDrawBuffers() noexcept;
    void disableDrawBuffers() noexcept;

    /** Multisampling control */
    void setSampleCount(int sampleCount) noexcept;
    int getSampleCount() const noexcept;

    /** Resolve (blit multisampled renderbuffers to original textures) */
    void resolve() noexcept;

    /** Layered rendering support */
    void setColorAttachmentTarget(int attachmentIndex, int layer = 0, int face = 0, int level = 0) noexcept;
    void setDepthAttachmentTarget(int layer = 0, int face = 0, int level = 0) noexcept;

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
    util::DynamicArray<const Texture*> mColorAttachments;
    util::DynamicArray<GLuint> mColorRenderbuffers;     //< MSAA color renderbuffers
    GLuint mDepthRenderbuffer{0};                       //< MSAA depth/stencil renderbuffer
    const Texture* mDepthStencilAttachment{nullptr};
    int mSampleCount{0};

    /** Layer/face tracking */
    util::DynamicArray<AttachmentTarget> mColorTargets;
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
    : mColorAttachments(colorAttachments), mDepthStencilAttachment(depthStencilAttachment)
{
    if (mColorAttachments.empty()) {
        HP_INTERNAL_LOG(E, "GPU: Framebuffer requires at least one color attachment");
        return;
    }

    // Initialize color targets array
    mColorTargets.resize(mColorAttachments.size());

    // Validate color attachments
    for (size_t i = 0; i < mColorAttachments.size(); ++i) {
        if (!mColorAttachments[i] || !mColorAttachments[i]->isValid()) {
            HP_INTERNAL_LOG(E, "GPU: Invalid color attachment at index %zu", i);
            return;
        }
    }

    // Validate depth/stencil attachment if provided
    if (mDepthStencilAttachment && !mDepthStencilAttachment->isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Invalid depth/stencil attachment");
        return;
    }

    createResolveFramebuffer();

    if (isValid()) {
        enableAllDrawBuffers();
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
    , mDepthRenderbuffer(std::exchange(other.mDepthRenderbuffer, 0))
    , mDepthStencilAttachment(std::exchange(other.mDepthStencilAttachment, nullptr))
    , mSampleCount(std::exchange(other.mSampleCount, 0))
    , mColorTargets(std::move(other.mColorTargets))
    , mDepthTarget(other.mDepthTarget)
{ }

inline Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        // Clean up current resources
        destroyMultisampleFramebuffer();
        if (mResolveFramebuffer != 0) glDeleteFramebuffers(1, &mResolveFramebuffer);

        // Move from other
        mResolveFramebuffer = std::exchange(other.mResolveFramebuffer, 0);
        mMultisampleFramebuffer = std::exchange(other.mMultisampleFramebuffer, 0);
        mColorAttachments = std::move(other.mColorAttachments);
        mColorRenderbuffers = std::move(other.mColorRenderbuffers);
        mDepthRenderbuffer = std::exchange(other.mDepthRenderbuffer, 0);
        mDepthStencilAttachment = other.mDepthStencilAttachment;
        mSampleCount = other.mSampleCount;
        mColorTargets = std::move(other.mColorTargets);
        mDepthTarget = other.mDepthTarget;

        other.mDepthStencilAttachment = nullptr;
        other.mSampleCount = 0;
        other.mDepthTarget = {};
    }
    return *this;
}

inline bool Framebuffer::isValid() const noexcept
{
    return mResolveFramebuffer > 0;
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
    return mColorAttachments.empty() ? 0
        : (mColorAttachments[0] ? mColorAttachments[0]->width() : 0);
}

inline int Framebuffer::height() const noexcept
{
    return mColorAttachments.empty() ? 0
        : (mColorAttachments[0] ? mColorAttachments[0]->height() : 0);
}

inline HP_IVec2 Framebuffer::dimensions() const noexcept
{
    return HP_IVEC2(width(), height());
}

inline const Texture* Framebuffer::getColorAttachment(int index) const noexcept
{
    return mColorAttachments[index];
}

inline const Texture* Framebuffer::getDepthAttachment() const noexcept
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
        HP_INTERNAL_LOG(E, "GPU: Cannot set sample count on invalid framebuffer");
        return;
    }

    if (sampleCount < 0) {
        HP_INTERNAL_LOG(E, "GPU: Sample count cannot be negative");
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
        HP_INTERNAL_LOG(E, "GPU: Cannot set attachment target on invalid framebuffer");
        return;
    }

    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorAttachments.size()));

    const Texture* texture = mColorAttachments[attachmentIndex];
    if (!texture) {
        HP_INTERNAL_LOG(E, "GPU: Color attachment %d is null", attachmentIndex);
        return;
    }

    // Validate mipmap level
    SDL_assert(level >= 0 && level < texture->mipLevels());

    // Validate target
    GLenum target = texture->target();
    SDL_assert(target != GL_TEXTURE_2D);

    // Validate layer and face parameters
    if (target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(layer >= 0 && layer < texture->depth());
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
    if (!isValid() || !mDepthStencilAttachment) {
        HP_INTERNAL_LOG(E, "GPU: Cannot set depth attachment target on invalid framebuffer or no depth attachment");
        return;
    }

    // Validate mipmap level
    SDL_assert(level >= 0 && level < mDepthStencilAttachment->mipLevels());

    // Validate target
    GLenum target = mDepthStencilAttachment->target();
    SDL_assert(target != GL_TEXTURE_2D);

    // Validate layer and face parameters
    if (target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(layer >= 0 && layer < mDepthStencilAttachment->depth());
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
        HP_INTERNAL_LOG(E, "GPU: Failed to create resolve framebuffer object");
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
            HP_INTERNAL_LOG(E, "GPU: Failed to create multisampled framebuffer");
            mSampleCount = 0;
            return;
        }
    }

    createAndAttachMultisampleRenderbuffers();

    if (!checkFramebufferComplete(mMultisampleFramebuffer)) {
        HP_INTERNAL_LOG(E, "GPU: Multisampled framebuffer is not complete");
        destroyMultisampleFramebuffer();
        mSampleCount = 0;
    }
}

inline void Framebuffer::destroyMultisampleFramebuffer() noexcept
{
    if (!mColorRenderbuffers.empty()) {
        glDeleteRenderbuffers(static_cast<GLsizei>(mColorRenderbuffers.size()), mColorRenderbuffers.data());
        mColorRenderbuffers.clear();
    }
    if (mDepthRenderbuffer > 0) {
        glDeleteRenderbuffers(1, &mDepthRenderbuffer);
        mDepthRenderbuffer = 0;
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
        HP_INTERNAL_LOG(W, "GPU: Unknown depth/stencil format, using GL_DEPTH_ATTACHMENT");
        return GL_DEPTH_ATTACHMENT;
    }
}

} // namespace gpu

#endif // HP_GPU_FRAMEBUFFER_HPP
