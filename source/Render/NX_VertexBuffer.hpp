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

#include "../NX_InstanceBuffer.hpp"

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

    constexpr gpu::VertexAttribute iPosition {
        .location = 7,
        .size = 3,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vec3),
        .offset = 0,
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(0, 0, 0, 0),
        }
    };

    constexpr gpu::VertexAttribute iRotation {
        .location = 8,
        .size = 4,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vec4),
        .offset = 0,
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(0, 0, 0, 1),
        }
    };

    constexpr gpu::VertexAttribute iScale {
        .location = 9,
        .size = 3,
        .type = GL_FLOAT,
        .normalized = false,
        .stride = sizeof(NX_Vec3),
        .offset = 0,
        .divisor = 1,
        .defaultValue = {
            .vFloat = NX_VEC4(1, 1, 1, 1),
        }
    };

    constexpr gpu::VertexAttribute iColor {
        .location = 10,
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
        .location = 11,
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
                    iPosition,
                }
            },
            gpu::VertexBufferDesc
            {
                .buffer = nullptr,
                .attributes = {
                    iRotation,
                }
            },
            gpu::VertexBufferDesc
            {
                .buffer = nullptr,
                .attributes = {
                    iScale,
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
        { 1, instances.GetBuffer(NX_INSTANCE_POSITION) },
        { 2, instances.GetBuffer(NX_INSTANCE_ROTATION) },
        { 3, instances.GetBuffer(NX_INSTANCE_SCALE) },
        { 4, instances.GetBuffer(NX_INSTANCE_COLOR) },
        { 5, instances.GetBuffer(NX_INSTANCE_CUSTOM) }
    });
}

inline void NX_VertexBuffer::unbindInstances()
{
    mVAO.unbindVertexBuffers({
        1, 2, 3, 4, 5
    });
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
