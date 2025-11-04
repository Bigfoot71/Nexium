/* Buffer.hpp -- Allows high-level GPU buffer management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_BUFFER_HPP
#define NX_GPU_BUFFER_HPP

#include <NX/NX_Log.h>

#include <SDL3/SDL_assert.h>
#include <glad/gles2.h>

#include <utility>

namespace gpu {

/* === Declaration === */

class Buffer {
public:
    /** Constructors */
    Buffer() = default;
    Buffer(GLenum target, GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_STATIC_DRAW) noexcept;

    /** Destructor and Move semantics */
    ~Buffer() noexcept;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    /** Public interface */
    bool IsValid() const noexcept;
    GLuint GetID() const noexcept;
    GLenum GetTarget() const noexcept;
    GLsizeiptr GetSize() const noexcept;
    GLenum GetUsage() const noexcept;

    /** Data management */
    void Reserve(GLsizeiptr minSize, bool keepData) noexcept;                   // Reallocates buffer if its size is < minSize, optionally preserving existing data
    void Realloc(GLsizeiptr newSize, const void* data) noexcept;                // Reallocates buffer to newSize and uploads data, no data preservation guarantee
    void Realloc(GLsizeiptr newSize, bool keepData) noexcept;                   // Reallocates buffer to newSize, optionally preserving existing data

    bool Upload(const void* data) noexcept;                                     // Overwrite entire buffer content, keep current size
    bool Upload(GLintptr offset, GLsizeiptr size, const void* data) noexcept;   // Overwrite part of the buffer at given offset

    template<typename T>
    bool UploadObject(const T& data) noexcept;                                  // Overwrite entire buffer from offset 0 with provided data (size = sizeof(T))

    /** Memory mapping */
    template <typename T>
    T* Map(GLenum access = GL_MAP_WRITE_BIT) noexcept;
    void* Map(GLenum access = GL_MAP_WRITE_BIT) noexcept;

    template <typename T>
    T* MapRange(GLintptr offset, GLsizeiptr length, GLbitfield access = GL_MAP_WRITE_BIT) noexcept;
    void* MapRange(GLintptr offset, GLsizeiptr length, GLbitfield access = GL_MAP_WRITE_BIT) noexcept;

    bool Unmap() noexcept;

private:
    /** Member variables */
    GLuint mID{0};
    GLenum mTarget{GL_ARRAY_BUFFER};
    GLsizeiptr mSize{0};
    GLenum mUsage{GL_STATIC_DRAW};

    /** Utility functions */
    void createBuffer(const void* data, GLenum usage) noexcept;

    /** Static helpers */
    static bool IsValidTarget(GLenum target) noexcept;
    static bool IsValidUsage(GLenum usage) noexcept;
    static bool IsValidMapAccess(GLbitfield access) noexcept;
    static const char* TargetToString(GLenum target) noexcept;
    static const char* UsageToString(GLenum usage) noexcept;
};

/* === Public Implementation === */

inline Buffer::Buffer(GLenum target, GLsizeiptr size, const void* data, GLenum usage) noexcept
    : mTarget(target), mSize(size), mUsage(usage)
{
    SDL_assert(size > 0);

    if (!IsValidTarget(target)) {
        NX_LOG(E, "GPU: Invalid buffer target: 0x%x", target);
        return;
    }

    if (!IsValidUsage(usage)) {
        NX_LOG(E, "GPU: Invalid buffer usage: 0x%x", usage);
        return;
    }

    if (size <= 0) {
        NX_LOG(E, "GPU: Invalid buffer size: %lld", static_cast<long long>(size));
        return;
    }

    createBuffer(data, usage);
}

inline Buffer::~Buffer() noexcept
{
    if (mID != 0) {
        glDeleteBuffers(1, &mID);
    }
}

inline Buffer::Buffer(Buffer&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mTarget(other.mTarget)
    , mSize(other.mSize)
    , mUsage(other.mUsage)
{ }

inline Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    if (this != &other) {
        if (mID != 0) glDeleteBuffers(1, &mID);
        mID = std::exchange(other.mID, 0);
        mTarget = other.mTarget;
        mSize = other.mSize;
        mUsage = other.mUsage;
    }
    return *this;
}

inline bool Buffer::IsValid() const noexcept
{
    return (mID > 0);
}

inline GLuint Buffer::GetID() const noexcept
{
    return mID;
}

inline GLenum Buffer::GetTarget() const noexcept
{
    return mTarget;
}

inline GLsizeiptr Buffer::GetSize() const noexcept
{
    return mSize;
}

inline GLenum Buffer::GetUsage() const noexcept
{
    return mUsage;
}

inline void Buffer::Reserve(GLsizeiptr size, bool keepData) noexcept
{
    if (size > mSize) {
        Realloc(size, keepData);
    }
}

template<typename T>
inline bool Buffer::UploadObject(const T& data) noexcept
{
    static_assert(!std::is_pointer_v<T>, "Do not pass pointers to UploadObject<T>!");
    return Upload(0, sizeof(T), &data);
}

inline bool Buffer::Upload(const void* data) noexcept
{
    return Upload(0, mSize, data);
}

template <typename T>
inline T* Buffer::Map(GLenum access) noexcept
{
    return static_cast<T*>(Map(access));
}

template <typename T>
inline T* Buffer::MapRange(GLintptr offset, GLsizeiptr length, GLbitfield access) noexcept
{
    return static_cast<T*>(MapRange(offset, length, access));
}

/* === Private Implementation === */

inline bool Buffer::IsValidTarget(GLenum target) noexcept
{
    switch (target) {
    case GL_ARRAY_BUFFER:
    case GL_ELEMENT_ARRAY_BUFFER:
    case GL_COPY_READ_BUFFER:
    case GL_COPY_WRITE_BUFFER:
    case GL_PIXEL_PACK_BUFFER:
    case GL_PIXEL_UNPACK_BUFFER:
    case GL_TRANSFORM_FEEDBACK_BUFFER:
    case GL_UNIFORM_BUFFER:
    case GL_SHADER_STORAGE_BUFFER:
        return true;
    default:
        return false;
    }
}

inline bool Buffer::IsValidUsage(GLenum usage) noexcept
{
    switch (usage) {
    case GL_STREAM_DRAW:
    case GL_STREAM_READ:
    case GL_STREAM_COPY:
    case GL_STATIC_DRAW:
    case GL_STATIC_READ:
    case GL_STATIC_COPY:
    case GL_DYNAMIC_DRAW:
    case GL_DYNAMIC_READ:
    case GL_DYNAMIC_COPY:
        return true;
    default:
        return false;
    }
}

inline bool Buffer::IsValidMapAccess(GLbitfield access) noexcept
{
    // Must have at least one of READ or WRITE
    if (!(access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT))) {
        return false;
    }

    // Check for valid bits only
    constexpr GLbitfield validBits =
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT |
        GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
        GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

    return (access & ~validBits) == 0;
}

inline const char* Buffer::TargetToString(GLenum target) noexcept
{
    switch (target) {
    case GL_ARRAY_BUFFER: return "GL_ARRAY_BUFFER";
    case GL_ELEMENT_ARRAY_BUFFER: return "GL_ELEMENT_ARRAY_BUFFER";
    case GL_COPY_READ_BUFFER: return "GL_COPY_READ_BUFFER";
    case GL_COPY_WRITE_BUFFER: return "GL_COPY_WRITE_BUFFER";
    case GL_PIXEL_PACK_BUFFER: return "GL_PIXEL_PACK_BUFFER";
    case GL_PIXEL_UNPACK_BUFFER: return "GL_PIXEL_UNPACK_BUFFER";
    case GL_TRANSFORM_FEEDBACK_BUFFER: return "GL_TRANSFORM_FEEDBACK_BUFFER";
    case GL_UNIFORM_BUFFER: return "GL_UNIFORM_BUFFER";
    case GL_SHADER_STORAGE_BUFFER: return "GL_SHADER_STORAGE_BUFFER";
    default: return "Unknown";
    }
}

inline const char* Buffer::UsageToString(GLenum usage) noexcept
{
    switch (usage) {
    case GL_STREAM_DRAW: return "GL_STREAM_DRAW";
    case GL_STREAM_READ: return "GL_STREAM_READ";
    case GL_STREAM_COPY: return "GL_STREAM_COPY";
    case GL_STATIC_DRAW: return "GL_STATIC_DRAW";
    case GL_STATIC_READ: return "GL_STATIC_READ";
    case GL_STATIC_COPY: return "GL_STATIC_COPY";
    case GL_DYNAMIC_DRAW: return "GL_DYNAMIC_DRAW";
    case GL_DYNAMIC_READ: return "GL_DYNAMIC_READ";
    case GL_DYNAMIC_COPY: return "GL_DYNAMIC_COPY";
    default: return "Unknown";
    }
}

} // namespace gpu

#endif // NX_GPU_BUFFER_HPP
