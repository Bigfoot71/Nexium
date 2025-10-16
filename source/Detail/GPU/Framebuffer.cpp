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

void Framebuffer::setDrawBuffers(std::initializer_list<int> buffers) noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot set draw buffers on invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> glBuffers{};
    for (int i = 0; i < NX_MIN(glBuffers.capacity(), buffers.size()); i++) {
        glBuffers.push_back(GL_COLOR_ATTACHMENT0 + buffers.begin()[i]);
    }

    Pipeline::withFramebufferBind(renderId(), [&]() {
        glDrawBuffers(static_cast<GLsizei>(glBuffers.size()), glBuffers.begin());
    });
}

void Framebuffer::enableDrawBuffers() noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot enable draw buffers on invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> buffers{};
    for (int i = 0; i < mColorAttachments.size(); i++) {
        buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    Pipeline::withFramebufferBind(renderId(), [&]() {
        glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
    });
}

void Framebuffer::disableDrawBuffers() noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot disable draw buffers on invalid framebuffer");
        return;
    }

    GLenum none = GL_NONE;
    Pipeline::withFramebufferBind(renderId(), [&]() {
        glDrawBuffers(1, &none);
    });
}

void Framebuffer::invalidate(std::initializer_list<int> buffers) noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot invalidate an invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> glBuffers{};
    for (int i = 0; i < NX_MIN(glBuffers.capacity(), buffers.size()); i++) {
        if (buffers.begin()[i] >= 0) glBuffers.push_back(GL_COLOR_ATTACHMENT0 + buffers.begin()[i]);
        else glBuffers.push_back(getDepthStencilAttachment(mDepthStencilAttachment.internalFormat()));
    }

    Pipeline::withFramebufferBind(renderId(), [&]() {
        glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(glBuffers.size()), glBuffers.begin());
    });
}

void Framebuffer::invalidate() noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot invalidate an invalid framebuffer");
        return;
    }

    util::StaticArray<GLenum, 32> buffers{};
    for (int i = 0; i < mColorAttachments.size(); i++) {
        buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    if (mDepthStencilAttachment.isValid()) {
        buffers.push_back(getDepthStencilAttachment(
            mDepthStencilAttachment.internalFormat()
        ));
    }

    Pipeline::withFramebufferBind(renderId(), [&]() {
        glInvalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(buffers.size()), buffers.data());
    });
}

void Framebuffer::resolve() noexcept
{
    if (!isValid() || mSampleCount == 0 || mMultisampleFramebuffer == 0) {
        return; // Nothing to resolve
    }

    // Resolve each color attachment
    for (size_t i = 0; i < mColorAttachments.size(); ++i) {
        resolveColorAttachment(static_cast<int>(i));
    }

    // Resolve depth attachment if present
    if (mDepthStencilAttachment.isValid()) {
        resolveDepthAttachment();
    }

    // Invalidate multisample framebuffer
    invalidate();
}

/* === Private Implementation === */

void Framebuffer::attachTexturesToResolveFramebuffer() noexcept
{
    Pipeline::withFramebufferBind(mResolveFramebuffer, [&]()
    {
        for (size_t i = 0; i < mColorAttachments.size(); ++i) {
            updateColorAttachment(static_cast<int>(i), false);
        }

        if (mDepthStencilAttachment.isValid()) {
            updateDepthAttachment(false);
        }
    });
}

void Framebuffer::updateColorAttachment(int index, bool bind) noexcept
{
    SDL_assert(mColorAttachments[index].isValid());

    const TextureView& texture = mColorAttachments[index];
    GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index);
    GLenum target = texture.target();
    GLuint textureId = texture.id();

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
        Pipeline::withFramebufferBind(mResolveFramebuffer, action);
    }
    else {
        action();
    }
}

void Framebuffer::updateDepthAttachment(bool bind) noexcept
{
    SDL_assert(mDepthStencilAttachment.isValid());

    const TextureView& texture = mDepthStencilAttachment;
    GLenum attachment = getDepthStencilAttachment(texture.internalFormat());
    GLenum target = texture.target();
    GLuint textureId = texture.id();

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
        Pipeline::withFramebufferBind(mResolveFramebuffer, action);
    }
    else {
        action();
    }
}

void Framebuffer::resolveColorAttachment(int index) noexcept
{
    SDL_assert(mColorAttachments[index].isValid());

    int fbWidth = width();
    int fbHeight = height();

    // Update the resolve framebuffer attachment to current layer/face
    updateColorAttachment(index, true);

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

void Framebuffer::resolveDepthAttachment() noexcept
{
    SDL_assert(mDepthStencilAttachment.isValid());

    int fbWidth = width();
    int fbHeight = height();

    // Update the resolve framebuffer attachment to current layer/face
    updateDepthAttachment(true);

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

void Framebuffer::createAndAttachMultisampleRenderbuffers() noexcept
{
    if (mSampleCount <= 0) {
        return;
    }

    int fbWidth = width();
    int fbHeight = height();

    /* --- Generates renderbuffers if they haven't already been generated --- */

    if (mColorRenderbuffers.empty()) {
        mColorRenderbuffers.reset(mColorAttachments.size());
        mColorRenderbuffers.resize(mColorAttachments.size());
        glGenRenderbuffers(static_cast<GLsizei>(mColorRenderbuffers.size()), mColorRenderbuffers.data());
    }

    /* --- Generates depth renderbuffer if needed --- */

    if (mDepthStencilAttachment.isValid() && mDepthStencilRenderbuffer == 0) {
        glGenRenderbuffers(1, &mDepthStencilRenderbuffer);
    }

    /* --- Attach and configure multisample renderbuffers in the framebuffer --- */

    Pipeline::withFramebufferBind(mMultisampleFramebuffer, [&]()
    {
        /* --- Configure color renderbuffers --- */

        for (size_t i = 0; i < mColorAttachments.size(); ++i)
        {
            SDL_assert(mColorAttachments[i].isValid());

            glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffers[i]);
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, mSampleCount,
                mColorAttachments[i].internalFormat(),
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

        if (mDepthStencilAttachment.isValid() && mDepthStencilRenderbuffer > 0)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderbuffer);
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, mSampleCount,
                mDepthStencilAttachment.internalFormat(),
                fbWidth, fbHeight
            );

            GLenum attachment = getDepthStencilAttachment(mDepthStencilAttachment.internalFormat());
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, attachment,
                GL_RENDERBUFFER,
                mDepthStencilRenderbuffer
            );
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    });
}

bool Framebuffer::checkFramebufferComplete(GLuint framebuffer) noexcept
{
    GLenum status = GL_FRAMEBUFFER_COMPLETE;
    Pipeline::withFramebufferBind(framebuffer, [&]() {
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
        NX_INTERNAL_LOG(E, "GPU: Framebuffer incomplete: %s (0x%x)", statusStr, status);
        SDL_assert(false);
        return false;
    }

    return true;
}

} // namespace gpu
