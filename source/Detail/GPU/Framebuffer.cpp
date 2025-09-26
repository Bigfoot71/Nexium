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

#include "../Util/StaticArray.hpp"
#include "./Framebuffer.hpp"
#include "./Pipeline.hpp"
#include "Texture.hpp"

namespace gpu {

/* === Public Implementation === */

void Framebuffer::setDrawBuffers(std::initializer_list<GLenum> buffers) noexcept
{
    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot set draw buffers on invalid framebuffer");
        return;
    }

    Pipeline::withFramebufferBind(renderId(), [&]() {
        glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.begin());
    });
}

void Framebuffer::enableAllDrawBuffers() noexcept
{
    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot enable draw buffers on invalid framebuffer");
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
        HP_INTERNAL_LOG(E, "GPU: Cannot disable draw buffers on invalid framebuffer");
        return;
    }

    GLenum none = GL_NONE;
    Pipeline::withFramebufferBind(renderId(), [&]() {
        glDrawBuffers(1, &none);
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
    if (mDepthStencilAttachment) {
        resolveDepthAttachment();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* === Private Implementation === */

void Framebuffer::attachTexturesToResolveFramebuffer() noexcept
{
    Pipeline::withFramebufferBind(mResolveFramebuffer, [&]()
    {
        /* --- Attach color textures --- */

        for (size_t i = 0; i < mColorAttachments.size(); ++i) {
            updateColorAttachment(static_cast<int>(i), false);
        }

        /* --- Attach depth/stencil texture if provided --- */

        if (mDepthStencilAttachment) {
            updateDepthAttachment(false);
        }
    });
}

void Framebuffer::updateColorAttachment(int index, bool bind) noexcept
{
    if (!mColorAttachments[index]) {
        return;
    }

    const Texture* texture = mColorAttachments[index];
    GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index);
    GLenum target = texture->target();
    GLuint textureId = texture->id();

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
    if (!mDepthStencilAttachment) {
        return;
    }

    const Texture* texture = mDepthStencilAttachment;
    GLenum attachment = getDepthStencilAttachment(texture->internalFormat());
    GLenum target = texture->target();
    GLuint textureId = texture->id();

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
    if (!mColorAttachments[index]) {
        return;
    }

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
    if (!mDepthStencilAttachment) {
        return;
    }

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

    /* --- Resize renderbuffer arrays if needed --- */

    if (mColorRenderbuffers.size() != mColorAttachments.size()) {
        if (!mColorRenderbuffers.empty()) {
            glDeleteRenderbuffers(static_cast<GLsizei>(mColorRenderbuffers.size()), mColorRenderbuffers.data());
        }
        mColorRenderbuffers.resize(mColorAttachments.size());
        glGenRenderbuffers(static_cast<GLsizei>(mColorRenderbuffers.size()), mColorRenderbuffers.data());
    }

    /* --- Create depth renderbuffer if needed --- */

    if (mDepthStencilAttachment && mDepthRenderbuffer == 0) {
        glGenRenderbuffers(1, &mDepthRenderbuffer);
    }

    /* --- Attach and configure multisample renderbuffers in the framebuffer --- */

    Pipeline::withFramebufferBind(mMultisampleFramebuffer, [&]()
    {
        /* --- Configure color renderbuffers --- */

        for (size_t i = 0; i < mColorAttachments.size(); ++i) {
            if (!mColorAttachments[i]) {
                continue;
            }

            glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffers[i]);
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, mSampleCount,
                mColorAttachments[i]->internalFormat(),
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

        if (mDepthStencilAttachment && mDepthRenderbuffer > 0) {
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer);
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, mSampleCount,
                mDepthStencilAttachment->internalFormat(),
                fbWidth, fbHeight
            );

            GLenum attachment = getDepthStencilAttachment(mDepthStencilAttachment->internalFormat());
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, attachment,
                GL_RENDERBUFFER,
                mDepthRenderbuffer
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
        HP_INTERNAL_LOG(E, "GPU: Framebuffer incomplete: %s (0x%x)", statusStr, status);
        SDL_assert(false);
        return false;
    }

    return true;
}

} // namespace gpu
