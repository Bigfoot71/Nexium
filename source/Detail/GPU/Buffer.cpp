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

#include "./Buffer.hpp"
#include "./Pipeline.hpp"

namespace gpu {

/* === Public Implementation === */

void Buffer::realloc(GLsizeiptr size, const void* data) noexcept
{
    SDL_assert(size > 0);

    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot set data on invalid buffer");
        return;
    }

    if (size <= 0) {
        HP_INTERNAL_LOG(E, "GPU: Invalid buffer size: %lld", static_cast<long long>(size));
        return;
    }

    mSize = size;

    Pipeline::withBufferBind(mTarget, mID, [&]() {
        glBufferData(mTarget, size, data, mUsage);
        if (glGetError() != GL_NO_ERROR) {
            HP_INTERNAL_LOG(E, "GPU: Failed to set buffer data");
        }
    });
}

void Buffer::realloc(GLsizeiptr size, bool keepData) noexcept
{
    SDL_assert(size > 0);

    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot set data on invalid buffer");
        return;
    }

    if (size <= 0) {
        HP_INTERNAL_LOG(E, "GPU: Invalid buffer size: %lld", static_cast<long long>(size));
        return;
    }

    const GLsizeiptr oldSize = mSize;

    Pipeline::withBufferBind(mTarget, mID,
        [&]()
        {
            /* --- If no need to keep the data we call glBufferData with NULL --- */

            if (!keepData) {
                glBufferData(mTarget, size, nullptr, mUsage);
                if (glGetError() != GL_NO_ERROR) {
                    HP_INTERNAL_LOG(E, "GPU: Failed to set buffer data");
                    return;
                }
                mSize = size;
                return;
            }

            /* --- Create temporary buffer to keep the current data --- */

            const GLsizeiptr preserveSize = std::min(oldSize, size);

            GLuint tempBuffer;
            glGenBuffers(1, &tempBuffer);

            glBindBuffer(GL_COPY_READ_BUFFER, mID);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tempBuffer);

            glBufferData(GL_COPY_WRITE_BUFFER, preserveSize, nullptr, GL_STATIC_COPY);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, preserveSize);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                HP_INTERNAL_LOG(E, "GPU: Failed to copy buffer data to temp buffer (error: %d)", error);
                glDeleteBuffers(1, &tempBuffer);
                return;
            }

            /* --- Reallocate the main buffer with the new size --- */

            glBindBuffer(mTarget, mID);
            glBufferData(mTarget, size, nullptr, mUsage);

            error = glGetError();
            if (error != GL_NO_ERROR) {
                HP_INTERNAL_LOG(E, "GPU: Failed to reallocate buffer (error: %d)", error);
                glDeleteBuffers(1, &tempBuffer);
                return;
            }

            /* --- Copy preserved data from temporary buffer --- */

            glBindBuffer(GL_COPY_READ_BUFFER, tempBuffer);
            glBindBuffer(GL_COPY_WRITE_BUFFER, mID);

            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, preserveSize);

            error = glGetError();
            if (error != GL_NO_ERROR) {
                HP_INTERNAL_LOG(E, "GPU: Failed to restore buffer data (error: %d)", error);
            }

            glBindBuffer(GL_COPY_READ_BUFFER, 0);
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

            /* --- Clear the temporary buffer --- */

            glDeleteBuffers(1, &tempBuffer);

            if (error == GL_NO_ERROR) {
                mSize = size;
            }
        }
    );
}

bool Buffer::upload(GLintptr offset, GLsizeiptr size, const void* data) noexcept
{
    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot set sub data on invalid buffer");
        return false;
    }

    if (offset < 0 || size <= 0 || offset + size > mSize) {
        HP_INTERNAL_LOG(E, "GPU: Invalid buffer sub data range [%lld, %lld) for buffer size %lld",
                static_cast<long long>(offset),
                static_cast<long long>(offset + size),
                static_cast<long long>(mSize));
        return false;
    }

    if (!data) {
        HP_INTERNAL_LOG(E, "GPU: Buffer sub data cannot be null");
        return false;
    }

    Pipeline::withBufferBind(mTarget, mID, [&]() {
        glBufferSubData(mTarget, offset, size, data);
        if (glGetError() != GL_NO_ERROR) {
            HP_INTERNAL_LOG(E, "GPU: Failed to set buffer sub data");
        }
    });

    return true;
}

void* Buffer::map(GLenum access) noexcept
{
    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot map invalid buffer");
        return nullptr;
    }

    if (!isValidMapAccess(access)) {
        HP_INTERNAL_LOG(E, "GPU: Invalid map access: 0x%x", access);
        return nullptr;
    }

    void* ptr = nullptr;
    Pipeline::withBufferBind(mTarget, mID, [&]() {
        ptr = glMapBufferRange(mTarget, 0, mSize, access);
        if (!ptr) {
            HP_INTERNAL_LOG(E, "GPU: Failed to map buffer");
        }
    });

    return ptr;
}

void* Buffer::mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access) noexcept
{
    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot map range on invalid buffer");
        return nullptr;
    }

    if (offset < 0 || length <= 0 || offset + length > mSize) {
        HP_INTERNAL_LOG(E, "GPU: Invalid map range [%lld, %lld) for buffer size %lld",
                static_cast<long long>(offset),
                static_cast<long long>(offset + length),
                static_cast<long long>(mSize));
        return nullptr;
    }

    if (!isValidMapRangeAccess(access)) {
        HP_INTERNAL_LOG(E, "GPU: Invalid map range access: 0x%x", access);
        return nullptr;
    }

    void* ptr = nullptr;
    Pipeline::withBufferBind(mTarget, mID, [&]() {
        ptr = glMapBufferRange(mTarget, offset, length, access);
        if (!ptr) {
            HP_INTERNAL_LOG(E, "GPU: Failed to map buffer range");
        }
    });

    return ptr;
}

bool Buffer::unmap() noexcept
{
    if (!isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Cannot unmap invalid buffer");
        return false;
    }

    GLboolean result = GL_FALSE;
    Pipeline::withBufferBind(mTarget, mID, [&]() {
        result = glUnmapBuffer(mTarget);
        if (result == GL_FALSE) {
            HP_INTERNAL_LOG(W, "GPU: Buffer unmap returned GL_FALSE (data corrupted)");
        }
    });

    return (result == GL_TRUE);
}

/* === Private Implementation === */

void Buffer::createBuffer(const void* data, GLenum usage) noexcept
{
    glGenBuffers(1, &mID);
    if (mID == 0) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create buffer object");
        return;
    }

    Pipeline::withBufferBind(mTarget, mID, [&]() {
        glBufferData(mTarget, mSize, data, usage);
        if (glGetError() != GL_NO_ERROR) {
            HP_INTERNAL_LOG(E, "GPU: Failed to upload buffer data");
            glDeleteBuffers(1, &mID);
            mID = 0;
            return;
        }
    });
}

} // namespace gpu
