/* NX_VertexBuffer.hpp -- Implementation of the API for vertex buffers
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_VERTEX_BUFFER_3D_HPP
#define NX_VERTEX_BUFFER_3D_HPP

#include <NX/NX_Render.h>

#include "./NX_InstanceBuffer.hpp"

#include "../Detail/GPU/VertexArray.hpp"
#include "../Detail/GPU/Buffer.hpp"

/* === Declaration === */

class NX_VertexBuffer {
public:
    NX_VertexBuffer(const NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount);

    void bindInstances(const NX_InstanceBuffer& instances);
    void unbindInstances();

    const gpu::VertexArray& vao() const;
    const gpu::Buffer& vbo() const;
    const gpu::Buffer& ebo() const;

    gpu::VertexArray& vao();
    gpu::Buffer& vbo();
    gpu::Buffer& ebo();

private:
    gpu::VertexArray mVAO{};
    gpu::Buffer mVBO{};
    gpu::Buffer mEBO{};
};

/* === Public Impelmentation === */

inline NX_VertexBuffer::NX_VertexBuffer(const NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount)
{
    /* --- Create main buffers --- */

    mVBO = gpu::Buffer(GL_ARRAY_BUFFER, sizeof(NX_Vertex3D) * vCount, vertices, GL_STATIC_DRAW);

    gpu::Buffer* ebo = nullptr;
    if (indices != nullptr) {
        mEBO = gpu::Buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * iCount, indices, GL_STATIC_DRAW);
        ebo = &mEBO;
    }

    /* --- Define main attributes --- */

    constexpr gpu::VertexAttribute aPosition {
        .location = 0,
        .size = 3,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vertex3D),
        .offset = offsetof(NX_Vertex3D, position),
        .divisor = 0
    };

    constexpr gpu::VertexAttribute aTexCoord {
        .location = 1,
        .size = 2,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vertex3D),
        .offset = offsetof(NX_Vertex3D, texcoord),
        .divisor = 0
    };

    constexpr gpu::VertexAttribute aNormal {
        .location = 2,
        .size = 3,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vertex3D),
        .offset = offsetof(NX_Vertex3D, normal),
        .divisor = 0
    };

    constexpr gpu::VertexAttribute aTangent {
        .location = 3,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vertex3D),
        .offset = offsetof(NX_Vertex3D, tangent),
        .divisor = 0
    };

    constexpr gpu::VertexAttribute aColor {
        .location = 4,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vertex3D),
        .offset = offsetof(NX_Vertex3D, color),
        .divisor = 0
    };

    constexpr gpu::VertexAttribute aBoneIds{
        .location = 5,
        .size = 4,
        .type = GL_INT,
        .normalized = false,
        .stride = sizeof(NX_Vertex3D),
        .offset = offsetof(NX_Vertex3D, boneIds),
        .divisor = 0
    };

    constexpr gpu::VertexAttribute aWeights {
        .location = 6,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vertex3D),
        .offset = offsetof(NX_Vertex3D, weights),
        .divisor = 0,
    };

    constexpr gpu::VertexAttribute iMatCol0 {
        .location = 7,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Mat4),
        .offset = offsetof(NX_Mat4, m00),
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(1, 0, 0, 0),
        }
    };

    constexpr gpu::VertexAttribute iMatCol1 {
        .location = 8,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Mat4),
        .offset = offsetof(NX_Mat4, m10),
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(0, 1, 0, 0),
        }
    };

    constexpr gpu::VertexAttribute iMatCol2 {
        .location = 9,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Mat4),
        .offset = offsetof(NX_Mat4, m20),
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(0, 0, 1, 0),
        }
    };

    constexpr gpu::VertexAttribute iMatCol3 {
        .location = 10,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Mat4),
        .offset = offsetof(NX_Mat4, m30),
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(0, 0, 0, 1),
        }
    };

    constexpr gpu::VertexAttribute iColor {
        .location = 11,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Color),
        .offset = 0,
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(1, 1, 1, 1),
        }
    };

    constexpr gpu::VertexAttribute iCustom {
        .location = 12,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Color),
        .offset = 0,
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(0, 0, 0, 0),
        }
    };

    /* --- Create vertex array --- */

    mVAO = gpu::VertexArray(ebo,
        {
            gpu::VertexBufferDesc
            {
                .buffer = &mVBO,
                .attributes = {
                    aPosition,
                    aTexCoord,
                    aNormal,
                    aTangent,
                    aColor,
                    aBoneIds,
                    aWeights,
                }
            },
            gpu::VertexBufferDesc
            {
                .buffer = nullptr,
                .attributes = {
                    iMatCol0,
                    iMatCol1,
                    iMatCol2,
                    iMatCol3,
                }
            },
            gpu::VertexBufferDesc
            {
                .buffer = nullptr,
                .attributes = {
                    iColor,
                }
            },
            gpu::VertexBufferDesc
            {
                .buffer = nullptr,
                .attributes = {
                    iCustom
                }
            }
        }
    );
}

inline void NX_VertexBuffer::bindInstances(const NX_InstanceBuffer& instances)
{
    mVAO.bindVertexBuffers({
        { 1, instances.getBuffer(NX_INSTANCE_DATA_MATRIX) },
        { 2, instances.getBuffer(NX_INSTANCE_DATA_COLOR) },
        { 3, instances.getBuffer(NX_INSTANCE_DATA_CUSTOM) }
    });
}

inline void NX_VertexBuffer::unbindInstances()
{
    mVAO.unbindVertexBuffers({ 1, 2, 3 });
}

inline const gpu::VertexArray& NX_VertexBuffer::vao() const
{
    return mVAO;
}

inline const gpu::Buffer& NX_VertexBuffer::vbo() const
{
    return mVBO;
}

inline const gpu::Buffer& NX_VertexBuffer::ebo() const
{
    return mEBO;
}

inline gpu::VertexArray& NX_VertexBuffer::vao()
{
    return mVAO;
}

inline gpu::Buffer& NX_VertexBuffer::vbo()
{
    return mVBO;
}

inline gpu::Buffer& NX_VertexBuffer::ebo()
{
    return mEBO;
}

#endif // NX_VERTEX_BUFFER_3D_HPP
