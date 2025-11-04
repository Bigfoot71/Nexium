/* Framebuffer.hpp -- Allows high-level GPU framebuffer management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_FRAMEBUFFER_HPP
#define NX_GPU_FRAMEBUFFER_HPP

#include "../../INX_BuildInfo.hpp"
#include "../Util/FixedArray.hpp"
#include "./TextureView.hpp"
#include "./Texture.hpp"

#include <NX/NX_Log.h>

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
    bool IsValid() const noexcept;
    GLuint GetResolveId() const noexcept;  // Always returns the resolve framebuffer (with original, sampleable textures)
    GLuint GetRenderId() const noexcept;   // Always returns the active framebuffer (MSAA or normal)
    int GetWidth() const noexcept;
    int GetHeight() const noexcept;
    NX_IVec2 GetDimensions() const noexcept;

    /** Attachment access (always the original textures) */
    const TextureView& GetColorAttachment(int index) const noexcept;
    const TextureView& GetDepthAttachment() const noexcept;
    size_t GetColorAttachmentCount() const noexcept;

    /** Draw buffers control */
    void SetDrawBuffers(std::initializer_list<int> buffers) noexcept;
    void EnableDrawBuffers() noexcept;
    void DisableDrawBuffers() noexcept;

    /** Invalidate content */
    void Invalidate(std::initializer_list<int> buffers) noexcept;
    void Invalidate() noexcept;

    /** Multisampling control */
    void SetSampleCount(int sampleCount) noexcept;
    int GetSampleCount() const noexcept;

    /** Resolve (blit multisampled renderbuffers to original textures) */
    void Resolve() noexcept;

    /** Layered rendering support */
    void SetColorAttachmentTarget(int attachmentIndex, int layer = 0, int face = 0, int level = 0) noexcept;
    void SetDepthAttachmentTarget(int layer = 0, int face = 0, int level = 0) noexcept;

    /** @warning These methods only accept depth and mip level changes */
    void UpdateColorTextureView(int attachmentIndex, const gpu::Texture& texture) noexcept;
    void UpdateDepthTextureView(const gpu::Texture& texture) noexcept;

    /** Get current layer/face targets */
    int GetColorAttachmentLayer(int attachmentIndex) const noexcept;
    int GetColorAttachmentFace(int attachmentIndex) const noexcept;
    int GetColorAttachmentLevel(int attachmentIndex) const noexcept;
    int GetDepthAttachmentLayer() const noexcept;
    int GetDepthAttachmentFace() const noexcept;
    int GetDepthAttachmentLevel() const noexcept;

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
    void CreateResolveFramebuffer() noexcept;
    void CreateMultisampleFramebuffer() noexcept;
    void DestroyMultisampleFramebuffer() noexcept;
    void AttachTexturesToResolveFramebuffer() noexcept;
    void CreateAndAttachMultisampleRenderbuffers() noexcept;
    bool CheckFramebufferComplete(GLuint framebuffer) noexcept;
    void UpdateColorAttachment(int index, bool bind) noexcept;
    void UpdateDepthAttachment(bool bind) noexcept;
    void ResolveColorAttachment(int index) noexcept;
    void ResolveDepthAttachment() noexcept;

    /** Static helpers */
    static GLenum GetDepthStencilAttachment(GLenum internalFormat) noexcept;
};

/* === Public Implementation === */

inline Framebuffer::Framebuffer(std::initializer_list<const Texture*> colorAttachments, const Texture* depthStencilAttachment) noexcept
{
    if (colorAttachments.size() == 0) {
        NX_LOG(E, "GPU: Framebuffer requires at least one color attachment");
        return;
    }

    /* --- Validate texture attachments (debug only) --- */

    if constexpr (INX_BuildInfo::debug) {
        NX_IVec2 expectedDims{};
        bool first = true;
        for (const Texture* tex : colorAttachments)  {
            SDL_assert(tex != nullptr);
            if (first) expectedDims = tex->GetDimensions(), first = false;
            else SDL_assert(tex->GetDimensions() == expectedDims);
        }
        if (depthStencilAttachment) {
            SDL_assert(depthStencilAttachment->GetDimensions() == expectedDims);
        }
    }

    /* --- Push all color attachments --- */

    if (!mColorAttachments.Reset(colorAttachments.size())) {
        NX_LOG(E, "GPU: Failed to allocate space to store color attachments IDs");
        return;
    }

    for (size_t i = 0; i < colorAttachments.size(); ++i) {
        mColorAttachments.EmplaceBack(*colorAttachments.begin()[i]);
    }

    /* --- Create color targets array --- */

    mColorTargets.Reset(colorAttachments.size());
    mColorTargets.Resize(colorAttachments.size());

    /* --- Validate depth/stencil attachment if provided and keep its ID --- */

    if (depthStencilAttachment) {
        if (!depthStencilAttachment->IsValid()) {
            NX_LOG(E, "GPU: Invalid depth/stencil attachment");
            return;
        }
        mDepthStencilAttachment = *depthStencilAttachment;
    }

    CreateResolveFramebuffer();

    if (IsValid()) {
        EnableDrawBuffers();
    }
}

inline Framebuffer::~Framebuffer() noexcept
{
    DestroyMultisampleFramebuffer();
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
        DestroyMultisampleFramebuffer();
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

inline bool Framebuffer::IsValid() const noexcept
{
    return (mResolveFramebuffer > 0);
}

inline GLuint Framebuffer::GetResolveId() const noexcept
{
    // Always returns the resolve framebuffer (with original textures)
    return mResolveFramebuffer;
}

inline GLuint Framebuffer::GetRenderId() const noexcept
{
    // Returns the active framebuffer for rendering (MSAA if enabled, otherwise the resolve framebuffer)
    return (mSampleCount > 0 && mMultisampleFramebuffer > 0) ? mMultisampleFramebuffer : mResolveFramebuffer;
}

inline int Framebuffer::GetWidth() const noexcept
{
    return mColorAttachments[0].GetWidth();
}

inline int Framebuffer::GetHeight() const noexcept
{
    return mColorAttachments[0].GetHeight();
}

inline NX_IVec2 Framebuffer::GetDimensions() const noexcept
{
    return mColorAttachments[0].GetDimensions();
}

inline const TextureView& Framebuffer::GetColorAttachment(int index) const noexcept
{
    return mColorAttachments[index];
}

inline const TextureView& Framebuffer::GetDepthAttachment() const noexcept
{
    return mDepthStencilAttachment;
}

inline size_t Framebuffer::GetColorAttachmentCount() const noexcept
{
    return mColorAttachments.GetSize();
}

inline void Framebuffer::SetSampleCount(int sampleCount) noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot set sample count on invalid framebuffer");
        return;
    }

    if (sampleCount < 0) {
        NX_LOG(E, "GPU: Sample count cannot be negative");
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
    CreateMultisampleFramebuffer();
}

inline int Framebuffer::GetSampleCount() const noexcept
{
    return mSampleCount;
}

inline void Framebuffer::SetColorAttachmentTarget(int attachmentIndex, int layer, int face, int level) noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot set attachment target on invalid framebuffer");
        return;
    }

    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorAttachments.GetSize()));
    const TextureView& texture = mColorAttachments[attachmentIndex];

    // Validate mipmap level
    SDL_assert(level >= 0 && level < texture.GetNumLevels());

    // Validate target
    GLenum target = texture.GetTarget();

    // Validate layer and face parameters
    if (target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(layer >= 0 && layer < texture.GetDepth());
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

    UpdateColorAttachment(attachmentIndex, true);
}

inline void Framebuffer::SetDepthAttachmentTarget(int layer, int face, int level) noexcept
{
    if (!IsValid() || !mDepthStencilAttachment.IsValid()) {
        NX_LOG(E, "GPU: Cannot set depth attachment target on invalid framebuffer or no depth attachment");
        return;
    }

    // Validate mipmap level
    SDL_assert(level >= 0 && level < mDepthStencilAttachment.GetNumLevels());

    // Validate target
    GLenum target = mDepthStencilAttachment.GetTarget();
    SDL_assert(target != GL_TEXTURE_2D);

    // Validate layer and face parameters
    if (target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        SDL_assert(layer >= 0 && layer < mDepthStencilAttachment.GetDepth());
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

    UpdateDepthAttachment(true);
}

inline void Framebuffer::UpdateColorTextureView(int attachmentIndex, const gpu::Texture& texture) noexcept
{
    // Only depth and mip count changes are accepted

    SDL_assert(texture.GetID() == mColorAttachments[attachmentIndex].GetID());
    SDL_assert(texture.GetTarget() == mColorAttachments[attachmentIndex].GetTarget());
    SDL_assert(texture.GetDimensions() == mColorAttachments[attachmentIndex].GetDimensions());
    SDL_assert(texture.GetInternalFormat() == mColorAttachments[attachmentIndex].GetInternalFormat());

    mColorAttachments[attachmentIndex] = gpu::TextureView(texture);
}

inline void Framebuffer::UpdateDepthTextureView(const gpu::Texture& texture) noexcept
{
    // Only depth and mip count changes are accepted

    SDL_assert(texture.GetID() == mDepthStencilAttachment.GetID());
    SDL_assert(texture.GetTarget() == mDepthStencilAttachment.GetTarget());
    SDL_assert(texture.GetDimensions() == mDepthStencilAttachment.GetDimensions());
    SDL_assert(texture.GetInternalFormat() == mDepthStencilAttachment.GetInternalFormat());

    mDepthStencilAttachment = gpu::TextureView(texture);
}

inline int Framebuffer::GetColorAttachmentLayer(int attachmentIndex) const noexcept
{
    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorTargets.GetSize()));
    return mColorTargets[attachmentIndex].layer;
}

inline int Framebuffer::GetColorAttachmentFace(int attachmentIndex) const noexcept
{
    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorTargets.GetSize()));
    return mColorTargets[attachmentIndex].face;
}

inline int Framebuffer::GetColorAttachmentLevel(int attachmentIndex) const noexcept
{
    SDL_assert(attachmentIndex >= 0 && attachmentIndex < static_cast<int>(mColorTargets.GetSize()));
    return mColorTargets[attachmentIndex].level;
}

inline int Framebuffer::GetDepthAttachmentLayer() const noexcept
{
    return mDepthTarget.layer;
}

inline int Framebuffer::GetDepthAttachmentFace() const noexcept
{
    return mDepthTarget.face;
}

inline int Framebuffer::GetDepthAttachmentLevel() const noexcept
{
    return mDepthTarget.level;
}

/* === Private Implementation === */

inline void Framebuffer::CreateResolveFramebuffer() noexcept
{
    glGenFramebuffers(1, &mResolveFramebuffer);
    if (mResolveFramebuffer == 0) {
        NX_LOG(E, "GPU: Failed to create resolve framebuffer object");
        return;
    }

    AttachTexturesToResolveFramebuffer();

    if (!CheckFramebufferComplete(mResolveFramebuffer)) {
        glDeleteFramebuffers(1, &mResolveFramebuffer);
        mResolveFramebuffer = 0;
        return;
    }
}

inline void Framebuffer::CreateMultisampleFramebuffer() noexcept
{
    if (mSampleCount <= 0) {
        return;
    }

    // Create multisampled framebuffer if needed
    if (mMultisampleFramebuffer == 0) {
        glGenFramebuffers(1, &mMultisampleFramebuffer);
        if (mMultisampleFramebuffer == 0) {
            NX_LOG(E, "GPU: Failed to create multisampled framebuffer");
            mSampleCount = 0;
            return;
        }
    }

    CreateAndAttachMultisampleRenderbuffers();

    if (!CheckFramebufferComplete(mMultisampleFramebuffer)) {
        NX_LOG(E, "GPU: Multisampled framebuffer is not complete");
        DestroyMultisampleFramebuffer();
        mSampleCount = 0;
    }

    EnableDrawBuffers();
}

inline void Framebuffer::DestroyMultisampleFramebuffer() noexcept
{
    if (!mColorRenderbuffers.IsEmpty()) {
        glDeleteRenderbuffers(static_cast<GLsizei>(mColorRenderbuffers.GetSize()), mColorRenderbuffers.GetData());
        mColorRenderbuffers.Clear();
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

inline GLenum Framebuffer::GetDepthStencilAttachment(GLenum internalFormat) noexcept
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
        NX_LOG(W, "GPU: Unknown depth/stencil format, using GL_DEPTH_ATTACHMENT");
        return GL_DEPTH_ATTACHMENT;
    }
}

} // namespace gpu

#endif // NX_GPU_FRAMEBUFFER_HPP
