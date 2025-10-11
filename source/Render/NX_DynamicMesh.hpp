#ifndef NX_RENDER_DYNAMIC_MESH_HPP
#define NX_RENDER_DYNAMIC_MESH_HPP

#include "../Detail/Util/DynamicArray.hpp"
#include "./NX_VertexBuffer.hpp"
#include <NX/NX_Render.h>
#include <NX/NX_Math.h>
#include <float.h>

/* === Declaration === */

class NX_DynamicMesh {
public:
    /** Public parameters */
    NX_ShadowCastMode shadowCastMode{NX_SHADOW_CAST_ENABLED};
    NX_ShadowFaceMode shadowFaceMode{NX_SHADOW_FACE_AUTO};
    NX_Layer layerMask{NX_LAYER_01};

public:
    NX_DynamicMesh(size_t initialCapacity);

    /** Pritimive update */
    void begin(NX_PrimitiveType type);
    void end();

    /** Vertices update */
    void setTexCoord(const NX_Vec2& texcoord);
    void setNormal(const NX_Vec3& normal);
    void setTangent(const NX_Vec4& tangent);
    void setColor(const NX_Color& color);
    void addVertex(const NX_Vec3& position);

    /** Getters */
    NX_PrimitiveType primitiveType() const;
    const NX_BoundingBox& aabb() const;
    NX_VertexBuffer& buffer() const;
    size_t vertexCount() const;

private:
    /** Buffers */
    util::DynamicArray<NX_Vertex3D> mVertices{};
    mutable NX_VertexBuffer mVertexBuffer;          /**< Mutable because we must be able to call bind/unbind 
                                                     *   for instantiated rendering without breaking
                                                     *   the entire 'NX_Draw' API.
                                                     */ 

    /** Current state */
    NX_BoundingBox mBoundingBox{};
    NX_Vertex3D mCurrentVertex{};
    NX_PrimitiveType mType{};
};

/* === Public Implementation === */

inline NX_DynamicMesh::NX_DynamicMesh(size_t initialCapacity)
    : mVertexBuffer(nullptr, initialCapacity * sizeof(NX_Vertex3D), nullptr, 0)
{
    if (!mVertices.reserve(initialCapacity)) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to reserve memory for immediate mesh");
    }
}

inline void NX_DynamicMesh::begin(NX_PrimitiveType type)
{
    mVertices.clear();
    mCurrentVertex = {
        .position = NX_VEC3_ZERO,
        .texcoord = NX_VEC2_ZERO,
        .normal = NX_VEC3_BACK,
        .tangent = NX_VEC4_IDENTITY,
        .color = NX_WHITE,
        .boneIds = NX_IVEC4_ZERO,
        .weights = NX_VEC4_ZERO
    };
    mType = type;
}

inline void NX_DynamicMesh::end()
{
    mVertexBuffer.vbo().reserve(mVertices.size() * sizeof(NX_Vertex3D), false);
    mVertexBuffer.vbo().upload(0, mVertices.size() * sizeof(NX_Vertex3D), mVertices.data());

    mBoundingBox.min = NX_VEC3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    mBoundingBox.max = NX_VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const NX_Vertex3D& vertex : mVertices) {
        mBoundingBox.min = NX_Vec3Min(mBoundingBox.min, vertex.position);
        mBoundingBox.max = NX_Vec3Max(mBoundingBox.max, vertex.position);
    }
}

inline void NX_DynamicMesh::setTexCoord(const NX_Vec2& texcoord)
{
    mCurrentVertex.texcoord = texcoord;
}

inline void NX_DynamicMesh::setNormal(const NX_Vec3& normal)
{
    mCurrentVertex.normal = normal;
}

inline void NX_DynamicMesh::setTangent(const NX_Vec4& tangent)
{
    mCurrentVertex.tangent = tangent;
}

inline void NX_DynamicMesh::setColor(const NX_Color& color)
{
    mCurrentVertex.color = color;
}

inline void NX_DynamicMesh::addVertex(const NX_Vec3& position)
{
    mCurrentVertex.position = position;
    mVertices.push_back(mCurrentVertex);
}

inline NX_PrimitiveType NX_DynamicMesh::primitiveType() const
{
    return mType;
}

inline const NX_BoundingBox& NX_DynamicMesh::aabb() const
{
    return mBoundingBox;
}

inline NX_VertexBuffer& NX_DynamicMesh::buffer() const
{
    return mVertexBuffer;
}

inline size_t NX_DynamicMesh::vertexCount() const
{
    return mVertices.size();
}

#endif // NX_RENDER_DYNAMIC_MESH_HPP
