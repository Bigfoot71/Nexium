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

#ifndef HP_GPU_VERTEX_ARRAY_HPP
#define HP_GPU_VERTEX_ARRAY_HPP

#include "../Util/DynamicArray.hpp"
#include "./Buffer.hpp"

#include <Hyperion/HP_Core.h>
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
};

struct VertexBufferDesc {
    Buffer* buffer;
    std::initializer_list<VertexAttribute> attributes;
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

    /** Access to buffers */
    size_t vertexBufferCount() const noexcept;
    Buffer* getVertexBuffer(size_t index) noexcept;
    const Buffer* getVertexBuffer(size_t index) const noexcept;
    Buffer* getIndexBuffer() noexcept;
    const Buffer* getIndexBuffer() const noexcept;

private:
    /** Member variables */
    GLuint mID{0};
    util::DynamicArray<Buffer*> mVertexBuffers;
    Buffer* mIndexBuffer{nullptr};

private:
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
    , mVertexBuffers(std::move(other.mVertexBuffers))
    , mIndexBuffer(other.mIndexBuffer)
{
    other.mIndexBuffer = nullptr;
}

inline VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other) {
        if (mID != 0) {
            glDeleteVertexArrays(1, &mID);
        }
        mID = std::exchange(other.mID, 0);
        mVertexBuffers = std::move(other.mVertexBuffers);
        mIndexBuffer = std::exchange(other.mIndexBuffer, nullptr);
    }
    return *this;
}

inline bool VertexArray::isValid() const noexcept
{
    return mID > 0;
}

inline GLuint VertexArray::id() const noexcept
{
    return mID;
}

inline size_t VertexArray::vertexBufferCount() const noexcept
{
    return mVertexBuffers.size();
}

inline Buffer* VertexArray::getVertexBuffer(size_t index) noexcept
{
    return mVertexBuffers[index];
}

inline const Buffer* VertexArray::getVertexBuffer(size_t index) const noexcept
{
    return mVertexBuffers[index];
}

inline Buffer* VertexArray::getIndexBuffer() noexcept
{
    return mIndexBuffer;
}

inline const Buffer* VertexArray::getIndexBuffer() const noexcept
{
    return mIndexBuffer;
}

/* === Private Static Implementation === */

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

#endif // HP_GPU_VERTEX_ARRAY_HPP
