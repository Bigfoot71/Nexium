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

#ifndef HP_VERTEX_BUFFER_3D_HPP
#define HP_VERTEX_BUFFER_3D_HPP

#include <Hyperion/HP_Render.h>

#include "../Detail/GPU/VertexArray.hpp"
#include "../Detail/GPU/Buffer.hpp"

/* === Declaration === */

class HP_VertexBuffer {
public:
    HP_VertexBuffer(const HP_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount);

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

inline HP_VertexBuffer::HP_VertexBuffer(const HP_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount)
{
    mVBO = gpu::Buffer(GL_ARRAY_BUFFER, sizeof(HP_Vertex3D) * vCount, vertices, GL_STATIC_DRAW);

    gpu::Buffer* ebo = nullptr;
    if (indices != nullptr) {
        mEBO = gpu::Buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * iCount, indices, GL_STATIC_DRAW);
        ebo = &mEBO;
    }

    mVAO = gpu::VertexArray(ebo,
        {
            gpu::VertexBufferDesc
            {
                .buffer = &mVBO,
                .attributes =
                {
                    gpu::VertexAttribute
                    {
                        .location = 0,
                        .size = 3,
                        .type = GL_FLOAT,
                        .normalized = false,
                        .stride = sizeof(HP_Vertex3D),
                        .offset = offsetof(HP_Vertex3D, position),
                        .divisor = 0
                    },
                    gpu::VertexAttribute
                    {
                        .location = 1,
                        .size = 2,
                        .type = GL_FLOAT,
                        .normalized = false,
                        .stride = sizeof(HP_Vertex3D),
                        .offset = offsetof(HP_Vertex3D, texcoord),
                        .divisor = 0
                    },
                    gpu::VertexAttribute
                    {
                        .location = 2,
                        .size = 3,
                        .type = GL_FLOAT,
                        .normalized = false,
                        .stride = sizeof(HP_Vertex3D),
                        .offset = offsetof(HP_Vertex3D, normal),
                        .divisor = 0
                    },
                    gpu::VertexAttribute
                    {
                        .location = 3,
                        .size = 4,
                        .type = GL_FLOAT,
                        .normalized = false,
                        .stride = sizeof(HP_Vertex3D),
                        .offset = offsetof(HP_Vertex3D, tangent),
                        .divisor = 0
                    },
                    gpu::VertexAttribute
                    {
                        .location = 4,
                        .size = 4,
                        .type = GL_FLOAT,
                        .normalized = false,
                        .stride = sizeof(HP_Vertex3D),
                        .offset = offsetof(HP_Vertex3D, color),
                        .divisor = 0
                    },
                    gpu::VertexAttribute
                    {
                        .location = 5,
                        .size = 4,
                        .type = GL_INT,
                        .normalized = false,
                        .stride = sizeof(HP_Vertex3D),
                        .offset = offsetof(HP_Vertex3D, boneIds),
                        .divisor = 0
                    },
                    gpu::VertexAttribute
                    {
                        .location = 6,
                        .size = 4,
                        .type = GL_FLOAT,
                        .normalized = false,
                        .stride = sizeof(HP_Vertex3D),
                        .offset = offsetof(HP_Vertex3D, weights),
                        .divisor = 0
                    },
                }
            }
        }
    );
}

inline const gpu::VertexArray& HP_VertexBuffer::vao() const
{
    return mVAO;
}

inline const gpu::Buffer& HP_VertexBuffer::vbo() const
{
    return mVBO;
}

inline const gpu::Buffer& HP_VertexBuffer::ebo() const
{
    return mEBO;
}

inline gpu::VertexArray& HP_VertexBuffer::vao()
{
    return mVAO;
}

inline gpu::Buffer& HP_VertexBuffer::vbo()
{
    return mVBO;
}

inline gpu::Buffer& HP_VertexBuffer::ebo()
{
    return mEBO;
}

#endif // HP_VERTEX_BUFFER_3D_HPP
