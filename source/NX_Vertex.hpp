#ifndef NX_VERTEX_HPP
#define NX_VERTEX_HPP

#include <NX/NX_Vertex.h>

#include "./Detail/GPU/VertexArray.hpp"
#include "./Detail/GPU/Buffer.hpp"
#include "./NX_InstanceBuffer.hpp"

// ============================================================================
// VERTEX BUFFER 3D
// ============================================================================

struct NX_VertexBuffer3D {
    /** Constructors */
    NX_VertexBuffer3D(const NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount);

    /** Delete copy */
    NX_VertexBuffer3D(const NX_VertexBuffer3D&) = delete;
    NX_VertexBuffer3D& operator=(const NX_VertexBuffer3D&) = delete;

    /** Move only */
    NX_VertexBuffer3D(NX_VertexBuffer3D&& other) noexcept;
    NX_VertexBuffer3D& operator=(NX_VertexBuffer3D&& other) noexcept;

    /** Instance buffers */
    void BindInstances(const NX_InstanceBuffer& instances);
    void UnbindInstances();

    /** Members */
    gpu::VertexArray vao{};
    gpu::Buffer vbo{};
    gpu::Buffer ebo{};
};

inline NX_VertexBuffer3D::NX_VertexBuffer3D(const NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount)
{
    /* --- Create main buffers --- */

    vbo = gpu::Buffer(GL_ARRAY_BUFFER, sizeof(NX_Vertex3D) * vCount, vertices, GL_STATIC_DRAW);

    if (indices != nullptr) {
        ebo = gpu::Buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * iCount, indices, GL_STATIC_DRAW);
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

    vao = gpu::VertexArray(
        ebo.IsValid() ? &ebo : nullptr,
        {
            gpu::VertexBufferDesc
            {
                .buffer = &vbo,
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

inline NX_VertexBuffer3D::NX_VertexBuffer3D(NX_VertexBuffer3D&& other) noexcept
    : vao(std::move(other.vao))
    , vbo(std::move(other.vbo))
    , ebo(std::move(other.ebo))
{ }

inline NX_VertexBuffer3D& NX_VertexBuffer3D::operator=(NX_VertexBuffer3D&& other) noexcept
{
    if (this != &other) {
        vao = std::move(other.vao);
        vbo = std::move(other.vbo);
        ebo = std::move(other.ebo);
    }
    return *this;
}

inline void NX_VertexBuffer3D::BindInstances(const NX_InstanceBuffer& instances)
{
    vao.BindVertexBuffers({
        { 1, instances.GetBuffer(NX_INSTANCE_POSITION) },
        { 2, instances.GetBuffer(NX_INSTANCE_ROTATION) },
        { 3, instances.GetBuffer(NX_INSTANCE_SCALE) },
        { 4, instances.GetBuffer(NX_INSTANCE_COLOR) },
        { 5, instances.GetBuffer(NX_INSTANCE_CUSTOM) }
    });
}

inline void NX_VertexBuffer3D::UnbindInstances()
{
    vao.UnbindVertexBuffers({
        1, 2, 3, 4, 5
    });
}

#endif // NX_VERTEX_HPP
