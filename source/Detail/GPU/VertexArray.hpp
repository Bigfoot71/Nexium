/* VertexArray.hpp -- Allows high-level GPU vertex array (VAO) management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_VERTEX_ARRAY_HPP
#define NX_GPU_VERTEX_ARRAY_HPP

#include "../Util/FixedArray.hpp"
#include "./Buffer.hpp"

#include <NX/NX_Core.h>
#include <glad/gles2.h>

#include <initializer_list>
#include <utility>

namespace gpu {

/* === Forward Declaration === */

class Buffer;

/* === Structures === */

struct VertexAttribute {
    GLuint location;       // Shader attribute location
    GLint size;            // 1, 2, 3, or 4 components
    GLenum type;           // GL_FLOAT, GL_INT, GL_UNSIGNED_INT, etc.
    GLboolean normalized;  // Normalization (ignored for integer attributes)
    GLsizei stride;        // Stride between vertices
    GLintptr offset;       // Offset in the buffer
    GLuint divisor;        // For instancing (0 = per vertex, >0 = per instance)
    union {
        NX_IVec4 vInt;
        NX_Vec4 vFloat;
    } defaultValue;
};

struct VertexBufferDesc {
    Buffer* buffer;
    std::initializer_list<VertexAttribute> attributes;
};

struct VertexBufferState {
    const Buffer* attachedBuffer{nullptr};
    util::FixedArray<VertexAttribute> attributes;
};

/* === Declaration === */

class VertexArray {
public:
    /** Constructors */
    VertexArray() = default;
    VertexArray(std::initializer_list<VertexBufferDesc> vertexBuffers) noexcept;
    VertexArray(Buffer* indexBuffer, std::initializer_list<VertexBufferDesc> vertexBuffers) noexcept;

    /** Destructor and Move semantics */
    ~VertexArray() noexcept;
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    /** Public interface */
    bool isValid() const noexcept;
    GLuint id() const noexcept;

    /** Access to index buffer */
    const Buffer* indexBuffer() const noexcept;

    /** Access to vertex buffers */
    size_t vertexBufferCount() const noexcept;
    const Buffer* vertexBuffer(size_t index) const noexcept;
    bool hasVertexBuffer(size_t index) const noexcept;
    void bindVertexBuffer(size_t index, const Buffer* buffer) noexcept;
    void unbindVertexBuffer(size_t index) noexcept;
    void bindVertexBuffers(std::initializer_list<std::pair<size_t, const Buffer*>> buffers) noexcept;
    void unbindVertexBuffers(std::initializer_list<size_t> indices) noexcept;

private:
    /** Member variables */
    GLuint mID{0};
    const Buffer* mIndexBuffer{nullptr};
    util::FixedArray<VertexBufferState> mVertexBuffers;

private:
    /** Private helpers */
    void applyDefaultAttribute(const VertexAttribute& attr) noexcept;
    void setupVertexAttribute(const VertexAttribute& attr) noexcept;

    /** Static helpers */
    static bool isValidAttributeSize(GLint size) noexcept;
    static bool isValidAttributeType(GLenum type) noexcept;
    static bool isIntegerAttributeType(GLenum type) noexcept;
    static const char* attributeTypeToString(GLenum type) noexcept;
};

/* === Public Implementation === */

inline VertexArray::VertexArray(std::initializer_list<VertexBufferDesc> vertexBuffers) noexcept
    : VertexArray(nullptr, vertexBuffers)
{ }

inline VertexArray::~VertexArray() noexcept
{
    if (mID != 0) {
        glDeleteVertexArrays(1, &mID);
    }
}

inline VertexArray::VertexArray(VertexArray&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mIndexBuffer(std::exchange(other.mIndexBuffer, nullptr))
    , mVertexBuffers(std::move(other.mVertexBuffers))
{ }

inline VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other) {
        if (mID != 0) {
            glDeleteVertexArrays(1, &mID);
        }
        mID = std::exchange(other.mID, 0);
        mIndexBuffer = std::exchange(other.mIndexBuffer, nullptr);
        mVertexBuffers = std::move(other.mVertexBuffers);
    }
    return *this;
}

inline bool VertexArray::isValid() const noexcept
{
    return (mID > 0);
}

inline GLuint VertexArray::id() const noexcept
{
    return mID;
}

inline const Buffer* VertexArray::indexBuffer() const noexcept
{
    return mIndexBuffer;
}

inline size_t VertexArray::vertexBufferCount() const noexcept
{
    return mVertexBuffers.size();
}

inline const Buffer* VertexArray::vertexBuffer(size_t index) const noexcept
{
    return mVertexBuffers[index].attachedBuffer;
}

inline bool VertexArray::hasVertexBuffer(size_t index) const noexcept
{
    return (mVertexBuffers[index].attachedBuffer != nullptr);
}

/* === Private Implementation === */

inline void VertexArray::applyDefaultAttribute(const VertexAttribute& attr) noexcept
{
    glDisableVertexAttribArray(attr.location);

    if (isIntegerAttributeType(attr.type)) {
        glVertexAttribI4iv(attr.location, attr.defaultValue.vInt.v);
    }
    else {
        glVertexAttrib4fv(attr.location, attr.defaultValue.vFloat.v);
    }

    if (attr.divisor > 0) {
        glVertexAttribDivisor(attr.location, attr.divisor);
    }
}

inline void VertexArray::setupVertexAttribute(const VertexAttribute& attr) noexcept
{
    glEnableVertexAttribArray(attr.location);

    if (isIntegerAttributeType(attr.type) && attr.normalized == GL_FALSE) {
        glVertexAttribIPointer(
            attr.location, attr.size, attr.type, attr.stride,
            reinterpret_cast<const void*>(attr.offset)
        );
    }
    else {
        glVertexAttribPointer(
            attr.location, attr.size, attr.type, attr.normalized,
            attr.stride, reinterpret_cast<const void*>(attr.offset)
        );
    }

    if (attr.divisor > 0) {
        glVertexAttribDivisor(attr.location, attr.divisor);
    }
}

inline bool VertexArray::isValidAttributeSize(GLint size) noexcept
{
    return (size >= 1 && size <= 4);
}

inline bool VertexArray::isValidAttributeType(GLenum type) noexcept
{
    switch (type) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_HALF_FLOAT:
    case GL_FLOAT:
    case GL_FIXED:
    case GL_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        return true;
    default:
        return false;
    }
}

inline bool VertexArray::isIntegerAttributeType(GLenum type) noexcept
{
    switch (type) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
        return true;
    default:
        return false;
    }
}

inline const char* VertexArray::attributeTypeToString(GLenum type) noexcept
{
    switch (type) {
    case GL_BYTE: return "GL_BYTE";
    case GL_UNSIGNED_BYTE: return "GL_UNSIGNED_BYTE";
    case GL_SHORT: return "GL_SHORT";
    case GL_UNSIGNED_SHORT: return "GL_UNSIGNED_SHORT";
    case GL_INT: return "GL_INT";
    case GL_UNSIGNED_INT: return "GL_UNSIGNED_INT";
    case GL_HALF_FLOAT: return "GL_HALF_FLOAT";
    case GL_FLOAT: return "GL_FLOAT";
    case GL_FIXED: return "GL_FIXED";
    case GL_INT_2_10_10_10_REV: return "GL_INT_2_10_10_10_REV";
    case GL_UNSIGNED_INT_2_10_10_10_REV: return "GL_UNSIGNED_INT_2_10_10_10_REV";
    default: return "Unknown";
    }
}

} // namespace gpu

#endif // NX_GPU_VERTEX_ARRAY_HPP
