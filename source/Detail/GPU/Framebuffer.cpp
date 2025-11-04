/* Framebuffer.cpp -- Allows high-level GPU framebuffer management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "../Util/StaticArray.hpp"
#include "./Framebuffer.hpp"
#include "./Pipeline.hpp"
#include "./Texture.hpp"

namespace gpu {

/* === Public Implementation === */

void Framebuffer::SetDrawBuffers(std::initializer_list<int> buffers) noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot set draw buffers on invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> glBuffers{};
    for (int i = 0; i < NX_MIN(glBuffers.GetCapacity(), buffers.size()); i++) {
        glBuffers.PushBack(GL_COLOR_ATTACHMENT0 + buffers.begin()[i]);
    }

    Pipeline::WithFramebufferBind(GetRenderId(), [&]() {
        glDrawBuffers(static_cast<GLsizei>(glBuffers.GetSize()), glBuffers.Begin());
    });
}

void Framebuffer::EnableDrawBuffers() noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot enable draw buffers on invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> buffers{};
    for (int i = 0; i < mColorAttachments.GetSize(); i++) {
        buffers.PushBack(GL_COLOR_ATTACHMENT0 + i);
    }

    Pipeline::WithFramebufferBind(GetRenderId(), [&]() {
        glDrawBuffers(static_cast<GLsizei>(buffers.GetSize()), buffers.GetData());
    });
}

void Framebuffer::DisableDrawBuffers() noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot disable draw buffers on invalid framebuffer");
        return;
    }

    GLenum none = GL_NONE;
    Pipeline::WithFramebufferBind(GetRenderId(), [&]() {
        glDrawBuffers(1, &none);
    });
}

void Framebuffer::Invalidate(std::initializer_list<int> buffers) noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot invalidate an invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> glBuffers{};
    for (int i = 0; i < NX_MIN(glBuffers.GetCapacity(), buffers.size()); i++) {
        if (buffers.begin()[i] >= 0) glBuffers.PushBack(GL_COLOR_ATTACHMENT0 + buffers.begin()[i]);
        else glBuffers.PushBack(GetDepthStencilAttachment(mDepthStencilAttachment.GetInternalFormat()));
    }

    Pipeline::WithFramebufferBind(GetRenderId(), [&]() {
        glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(glBuffers.GetSize()), glBuffers.Begin());
    });
}

void Framebuffer::Invalidate() noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot invalidate an invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> buffers{};
    for (int i = 0; i < mColorAttachments.GetSize(); i++) {
        buffers.PushBack(GL_COLOR_ATTACHMENT0 + i);
    }

    if (mDepthStencilAttachment.IsValid()) {
        buffers.PushBack(GetDepthStencilAttachment(
            mDepthStencilAttachment.GetInternalFormat()
        ));
    }

    Pipeline::WithFramebufferBind(GetRenderId(), [&]() {
        glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(buffers.GetSize()), buffers.GetData());
    });
}

void Framebuffer::Resolve() noexcept
{
    if (!IsValid() || mSampleCount == 0 || mMultisampleFramebuffer == 0) {
        return; // Nothing to resolve
    }

    // Resolve each color attachment
    for (size_t i = 0; i < mColorAttachments.GetSize(); ++i) {
        ResolveColorAttachment(static_cast<int>(i));
    }

    // Resolve depth attachment if present
    if (mDepthStencilAttachment.IsValid()) {
        ResolveDepthAttachment();
    }

    // Invalidate multisample framebuffer
    Invalidate();
}

/* === Private Implementation === */

void Framebuffer::AttachTexturesToResolveFramebuffer() noexcept
{
    Pipeline::WithFramebufferBind(mResolveFramebuffer, [&]()
    {
        for (size_t i = 0; i < mColorAttachments.GetSize(); ++i) {
            UpdateColorAttachment(static_cast<int>(i), false);
        }

        if (mDepthStencilAttachment.IsValid()) {
            UpdateDepthAttachment(false);
        }
    });
}

void Framebuffer::UpdateColorAttachment(int index, bool bind) noexcept
{
    SDL_assert(mColorAttachments[index].IsValid());

    const TextureView& texture = mColorAttachments[index];
    GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index);
    GLenum target = texture.GetTarget();
    GLuint textureId = texture.GetID();

    const AttachmentTarget& targetInfo = mColorTargets[index];

    const auto action = [&]() {
        if (target == GL_TEXTURE_2D) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, textureId, targetInfo.level);
        }
        else if (target == GL_TEXTURE_2D_ARRAY) {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureId, targetInfo.level, targetInfo.layer);
        }
        else if (target == GL_TEXTURE_CUBE_MAP) {
            GLenum cubeTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + targetInfo.face;
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, cubeTarget, textureId, targetInfo.level);
        }
        else if (target == GL_TEXTURE_CUBE_MAP_ARRAY) {
            int layerFace = targetInfo.layer * 6 + targetInfo.face;
            glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureId, targetInfo.level, layerFace);
        }
    };

    if (bind) {
        Pipeline::WithFramebufferBind(mResolveFramebuffer, action);
    }
    else {
        action();
    }
}

void Framebuffer::UpdateDepthAttachment(bool bind) noexcept
{
    SDL_assert(mDepthStencilAttachment.IsValid());

    const TextureView& texture = mDepthStencilAttachment;
    GLenum attachment = GetDepthStencilAttachment(texture.GetInternalFormat());
    GLenum target = texture.GetTarget();
    GLuint textureId = texture.GetID();

    const AttachmentTarget& targetInfo = mDepthTarget;

    const auto action = [&]() {
        if (target == GL_TEXTURE_2D) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, textureId, targetInfo.level);
        }
        else if (target == GL_TEXTURE_2D_ARRAY) {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureId, targetInfo.level, targetInfo.layer);
        }
        else if (target == GL_TEXTURE_CUBE_MAP) {
            GLenum cubeTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + targetInfo.face;
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, cubeTarget, textureId, targetInfo.level);
        }
        else if (target == GL_TEXTURE_CUBE_MAP_ARRAY) {
            int layerFace = targetInfo.layer * 6 + targetInfo.face;
            glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureId, targetInfo.level, layerFace);
        }
    };

    if (bind) {
        Pipeline::WithFramebufferBind(mResolveFramebuffer, action);
    }
    else {
        action();
    }
}

void Framebuffer::ResolveColorAttachment(int index) noexcept
{
    SDL_assert(mColorAttachments[index].IsValid());

    int fbWidth = GetWidth();
    int fbHeight = GetHeight();

    // Update the resolve framebuffer attachment to current layer/face
    UpdateColorAttachment(index, true);

    // Bind multisampled framebuffer as read
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mMultisampleFramebuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index));

    // Bind resolve framebuffer as draw
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFramebuffer);
    GLenum drawBuffer = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index);
    glDrawBuffers(1, &drawBuffer);

    // Blit from MSAA renderbuffer to the specific layer/face
    glBlitFramebuffer(
        0, 0, fbWidth, fbHeight,
        0, 0, fbWidth, fbHeight,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
}

void Framebuffer::ResolveDepthAttachment() noexcept
{
    SDL_assert(mDepthStencilAttachment.IsValid());

    int fbWidth = GetWidth();
    int fbHeight = GetHeight();

    // Update the resolve framebuffer attachment to current layer/face
    UpdateDepthAttachment(true);

    // Bind multisampled framebuffer as read
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mMultisampleFramebuffer);

    // Bind resolve framebuffer as draw
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFramebuffer);

    // Blit depth buffer from MSAA renderbuffer to the specific layer/face
    glBlitFramebuffer(
        0, 0, fbWidth, fbHeight,
        0, 0, fbWidth, fbHeight,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
}

void Framebuffer::CreateAndAttachMultisampleRenderbuffers() noexcept
{
    if (mSampleCount <= 0) {
        return;
    }

    int fbWidth = GetWidth();
    int fbHeight = GetHeight();

    /* --- Generates renderbuffers if they haven't already been generated --- */

    if (mColorRenderbuffers.IsEmpty()) {
        mColorRenderbuffers.Reset(mColorAttachments.GetSize());
        mColorRenderbuffers.Resize(mColorAttachments.GetSize());
        glGenRenderbuffers(static_cast<GLsizei>(mColorRenderbuffers.GetSize()), mColorRenderbuffers.GetData());
    }

    /* --- Generates depth renderbuffer if needed --- */

    if (mDepthStencilAttachment.IsValid() && mDepthStencilRenderbuffer == 0) {
        glGenRenderbuffers(1, &mDepthStencilRenderbuffer);
    }

    /* --- Attach and configure multisample renderbuffers in the framebuffer --- */

    Pipeline::WithFramebufferBind(mMultisampleFramebuffer, [&]()
    {
        /* --- Configure color renderbuffers --- */

        for (size_t i = 0; i < mColorAttachments.GetSize(); ++i)
        {
            SDL_assert(mColorAttachments[i].IsValid());

            glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffers[i]);
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, mSampleCount,
                mColorAttachments[i].GetInternalFormat(),
                fbWidth, fbHeight
            );

            GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, attachment,
                GL_RENDERBUFFER,
                mColorRenderbuffers[i]
            );
        }

        /* --- Configure depth/stencil renderbuffer --- */

        if (mDepthStencilAttachment.IsValid() && mDepthStencilRenderbuffer > 0)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderbuffer);
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, mSampleCount,
                mDepthStencilAttachment.GetInternalFormat(),
                fbWidth, fbHeight
            );

            GLenum attachment = GetDepthStencilAttachment(mDepthStencilAttachment.GetInternalFormat());
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, attachment,
                GL_RENDERBUFFER,
                mDepthStencilRenderbuffer
            );
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    });
}

bool Framebuffer::CheckFramebufferComplete(GLuint framebuffer) noexcept
{
    GLenum status = GL_FRAMEBUFFER_COMPLETE;
    Pipeline::WithFramebufferBind(framebuffer, [&]() {
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    });

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        const char* statusStr = "Unknown";
        switch (status) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            statusStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            statusStr = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            statusStr = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        }
        NX_LOG(E, "GPU: Framebuffer incomplete: %s (0x%x)", statusStr, status);
        SDL_assert(false);
        return false;
    }

    return true;
}

} // namespace gpu
