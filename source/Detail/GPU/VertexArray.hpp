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

#include <NX/NX_Math.h>
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
    bool IsValid() const noexcept;
    GLuint GetID() const noexcept;

    /** Access to index buffer */
    const Buffer* GetIndexBuffer() const noexcept;

    /** Access to vertex buffers */
    size_t GetVertexBufferCount() const noexcept;
    const Buffer* GetVertexBuffer(size_t index) const noexcept;
    bool HasVertexBuffer(size_t index) const noexcept;
    void BindVertexBuffer(size_t index, const Buffer* buffer) noexcept;
    void UnbindVertexBuffer(size_t index) noexcept;
    void BindVertexBuffers(std::initializer_list<std::pair<size_t, const Buffer*>> buffers) noexcept;
    void UnbindVertexBuffers(std::initializer_list<size_t> indices) noexcept;

private:
    /** Member variables */
    GLuint mID{0};
    const Buffer* mIndexBuffer{nullptr};
    util::FixedArray<VertexBufferState> mVertexBuffers;

private:
    /** Private helpers */
    void ApplyDefaultAttribute(const VertexAttribute& attr) noexcept;
    void SetupVertexAttribute(const VertexAttribute& attr) noexcept;

    /** Static helpers */
    static bool IsValidAttributeSize(GLint size) noexcept;
    static bool IsValidAttributeType(GLenum type) noexcept;
    static bool IsIntegerAttributeType(GLenum type) noexcept;
    static const char* AttributeTypeToString(GLenum type) noexcept;
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

inline bool VertexArray::IsValid() const noexcept
{
    return (mID > 0);
}

inline GLuint VertexArray::GetID() const noexcept
{
    return mID;
}

inline const Buffer* VertexArray::GetIndexBuffer() const noexcept
{
    return mIndexBuffer;
}

inline size_t VertexArray::GetVertexBufferCount() const noexcept
{
    return mVertexBuffers.size();
}

inline const Buffer* VertexArray::GetVertexBuffer(size_t index) const noexcept
{
    return mVertexBuffers[index].attachedBuffer;
}

inline bool VertexArray::HasVertexBuffer(size_t index) const noexcept
{
    return (mVertexBuffers[index].attachedBuffer != nullptr);
}

/* === Private Implementation === */

inline void VertexArray::ApplyDefaultAttribute(const VertexAttribute& attr) noexcept
{
    glDisableVertexAttribArray(attr.location);

    if (IsIntegerAttributeType(attr.type)) {
        glVertexAttribI4iv(attr.location, attr.defaultValue.vInt.v);
    }
    else {
        glVertexAttrib4fv(attr.location, attr.defaultValue.vFloat.v);
    }

    if (attr.divisor > 0) {
        glVertexAttribDivisor(attr.location, attr.divisor);
    }
}

inline void VertexArray::SetupVertexAttribute(const VertexAttribute& attr) noexcept
{
    glEnableVertexAttribArray(attr.location);

    if (IsIntegerAttributeType(attr.type) && attr.normalized == GL_FALSE) {
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

inline bool VertexArray::IsValidAttributeSize(GLint size) noexcept
{
    return (size >= 1 && size <= 4);
}

inline bool VertexArray::IsValidAttributeType(GLenum type) noexcept
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

inline bool VertexArray::IsIntegerAttributeType(GLenum type) noexcept
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

inline const char* VertexArray::AttributeTypeToString(GLenum type) noexcept
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
