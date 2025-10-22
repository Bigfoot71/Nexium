/* Buffer.cpp -- Allows high-level GPU buffer management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./Buffer.hpp"
#include "./Pipeline.hpp"

namespace gpu {

/* === Public Implementation === */

void Buffer::realloc(GLsizeiptr newSize, const void* data) noexcept
{
    SDL_assert(newSize > 0);

    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot realloc an invalid buffer (id=%u)", mID);
        return;
    }

    if (newSize <= 0) {
        NX_INTERNAL_LOG(E, "GPU: Invalid buffer size: %lld", static_cast<long long>(newSize));
        return;
    }

    Pipeline::withBufferBind(mTarget, mID, [&]()
    {
        if (newSize != mSize) {
            glBufferData(mTarget, newSize, data, mUsage);
            mSize = newSize;
        }
        else {
            glBufferData(mTarget, mSize, nullptr, mUsage);
            if (data) {
                glBufferSubData(mTarget, 0, mSize, data);
            }
        }

        const GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            NX_INTERNAL_LOG(E, "GPU: Buffer (id=%u) realloc failed (error 0x%04X, size=%lld)",
                            mID, err, static_cast<long long>(newSize));
        }
    });
}

void Buffer::realloc(GLsizeiptr newSize, bool keepData) noexcept
{
    SDL_assert(newSize > 0);

    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot realloc an invalid buffer (id=%u)", mID);
        return;
    }

    if (newSize <= 0) {
        NX_INTERNAL_LOG(E, "GPU: Invalid buffer size: %lld", static_cast<long long>(newSize));
        return;
    }

    if (newSize == mSize) {
        return;
    }

    const GLsizeiptr oldSize = mSize;

    Pipeline::withBufferBind(mTarget, mID, [&]()
    {
        const GLsizeiptr preserveSize = std::min(oldSize, newSize);

        /* --- Case without data preservation --- */

        if (!keepData || preserveSize <= 0) {
            glBufferData(mTarget, newSize, nullptr, mUsage);
            const GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                NX_INTERNAL_LOG(E, "GPU: Buffer realloc failed (id=%u, error=0x%04X, size=%lld)",
                                mID, err, static_cast<long long>(newSize));
                return;
            }
            mSize = newSize;
            return;
        }

        /* --- Cases with data preservation --- */

        GLuint tempBuffer = 0;
        glGenBuffers(1, &tempBuffer);
        glBindBuffer(GL_COPY_READ_BUFFER, mID);
        glBindBuffer(GL_COPY_WRITE_BUFFER, tempBuffer);

        glBufferData(GL_COPY_WRITE_BUFFER, preserveSize, nullptr, GL_STATIC_COPY);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, preserveSize);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            NX_INTERNAL_LOG(E, "GPU: Failed to copy buffer data to temp buffer (id=%u, err=0x%04X, size=%lld)",
                            mID, err, static_cast<long long>(preserveSize));
            glDeleteBuffers(1, &tempBuffer);
            return;
        }

        /* --- Main reallocation --- */

        glBindBuffer(mTarget, mID);
        glBufferData(mTarget, newSize, nullptr, mUsage);

        err = glGetError();
        if (err != GL_NO_ERROR) {
            NX_INTERNAL_LOG(E, "GPU: Failed to realloc main buffer (id=%u, err=0x%04X, newSize=%lld)",
                            mID, err, static_cast<long long>(newSize));
            glDeleteBuffers(1, &tempBuffer);
            return;
        }

        /* --- Copy back from temporary buffer --- */

        glBindBuffer(GL_COPY_READ_BUFFER, tempBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mID);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, preserveSize);

        err = glGetError();
        if (err != GL_NO_ERROR) {
            NX_INTERNAL_LOG(E, "GPU: Failed to restore preserved data (id=%u, err=0x%04X, preserved=%lld)",
                            mID, err, static_cast<long long>(preserveSize));
        }
        else {
            mSize = newSize;
        }

        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
        glDeleteBuffers(1, &tempBuffer);
    });
}

bool Buffer::upload(GLintptr offset, GLsizeiptr size, const void* data) noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot set sub data on invalid buffer");
        return false;
    }

    if (offset < 0 || size <= 0 || offset + size > mSize) {
        NX_INTERNAL_LOG(E, "GPU: Invalid buffer sub data range [%lld, %lld) for buffer size %lld",
                static_cast<long long>(offset),
                static_cast<long long>(offset + size),
                static_cast<long long>(mSize));
        return false;
    }

    if (!data) {
        NX_INTERNAL_LOG(E, "GPU: Buffer sub data cannot be null");
        return false;
    }

    Pipeline::withBufferBind(mTarget, mID, [&]() {
        glBufferSubData(mTarget, offset, size, data);
        if (glGetError() != GL_NO_ERROR) {
            NX_INTERNAL_LOG(E, "GPU: Failed to set buffer sub data");
        }
    });

    return true;
}

void* Buffer::map(GLenum access) noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot map invalid buffer");
        return nullptr;
    }

    if (!isValidMapAccess(access)) {
        NX_INTERNAL_LOG(E, "GPU: Invalid map access: 0x%x", access);
        return nullptr;
    }

    void* ptr = nullptr;
    Pipeline::withBufferBind(mTarget, mID, [&]() {
        ptr = glMapBufferRange(mTarget, 0, mSize, access);
        if (!ptr) {
            NX_INTERNAL_LOG(E, "GPU: Failed to map buffer");
        }
    });

    return ptr;
}

void* Buffer::mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access) noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot map range on invalid buffer");
        return nullptr;
    }

    if (offset < 0 || length <= 0 || offset + length > mSize) {
        NX_INTERNAL_LOG(E, "GPU: Invalid map range [%lld, %lld) for buffer size %lld",
                static_cast<long long>(offset),
                static_cast<long long>(offset + length),
                static_cast<long long>(mSize));
        return nullptr;
    }

    if (!isValidMapAccess(access)) {
        NX_INTERNAL_LOG(E, "GPU: Invalid map range access: 0x%x", access);
        return nullptr;
    }

    void* ptr = nullptr;
    Pipeline::withBufferBind(mTarget, mID, [&]() {
        ptr = glMapBufferRange(mTarget, offset, length, access);
        if (!ptr) {
            NX_INTERNAL_LOG(E, "GPU: Failed to map buffer range");
        }
    });

    return ptr;
}

bool Buffer::unmap() noexcept
{
    if (!isValid()) {
        NX_INTERNAL_LOG(E, "GPU: Cannot unmap invalid buffer");
        return false;
    }

    GLboolean result = GL_FALSE;
    Pipeline::withBufferBind(mTarget, mID, [&]() {
        result = glUnmapBuffer(mTarget);
        if (result == GL_FALSE) {
            NX_INTERNAL_LOG(W, "GPU: Buffer unmap returned GL_FALSE (data corrupted)");
        }
    });

    return (result == GL_TRUE);
}

/* === Private Implementation === */

void Buffer::createBuffer(const void* data, GLenum usage) noexcept
{
    glGenBuffers(1, &mID);
    if (mID == 0) {
        NX_INTERNAL_LOG(E, "GPU: Failed to create buffer object");
        return;
    }

    Pipeline::withBufferBind(mTarget, mID, [&]() {
        glBufferData(mTarget, mSize, data, usage);
        if (glGetError() != GL_NO_ERROR) {
            NX_INTERNAL_LOG(E, "GPU: Failed to upload buffer data");
            glDeleteBuffers(1, &mID);
            mID = 0;
            return;
        }
    });
}

} // namespace gpu
