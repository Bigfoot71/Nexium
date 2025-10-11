/* PoolMesh.hpp -- Storage pool for meshes and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_POOL_MESH_HPP
#define NX_RENDER_POOL_MESH_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../NX_InstanceBuffer.hpp"
#include "../NX_VertexBuffer.hpp"
#include <NX/NX_Render.h>
#include <cfloat>

namespace render {

/* === Declaration === */

class PoolMesh {
public:
    /** Instance buffer functions */
    NX_InstanceBuffer* createInstanceBuffer(NX_InstanceData bitfield, size_t count);
    void destroyInstanceBuffer(NX_InstanceBuffer* buffer);

    /** Mesh functions */
    NX_Mesh* createMesh(NX_PrimitiveType type, NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, const NX_BoundingBox& aabb, bool upload);
    NX_Mesh* createMesh(NX_PrimitiveType type, NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, bool upload);
    void destroyMesh(NX_Mesh* mesh);
    void updateMesh(NX_Mesh* mesh);

private:
    util::ObjectPool<NX_InstanceBuffer, 128> mInstanceBuffers{};
    util::ObjectPool<NX_VertexBuffer, 512> mVertexBuffers{};
    util::ObjectPool<NX_Mesh, 512> mMeshes{};
};

/* === Public Implementation === */

inline NX_InstanceBuffer* PoolMesh::createInstanceBuffer(NX_InstanceData bitfield, size_t count)
{
    NX_InstanceBuffer* instances = mInstanceBuffers.create(bitfield, count);
    if (!instances) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to create instance buffer; Object pool issue");
        return nullptr;
    }

    return instances;
}

inline void PoolMesh::destroyInstanceBuffer(NX_InstanceBuffer* buffer)
{
    mInstanceBuffers.destroy(buffer);
}

inline NX_Mesh* PoolMesh::createMesh(NX_PrimitiveType type, NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, const NX_BoundingBox& aabb, bool upload)
{
    SDL_assert(vertices != nullptr && vCount > 0);

    /* --- Reserve the object in the pool --- */

    NX_Mesh* mesh = mMeshes.create();
    if (mesh == nullptr) {
        NX_INTERNAL_LOG(E, "RENDER: Failed to load mesh; Object pool issue");
        return nullptr;
    }

    /* --- Creating the GPU vertex buffer --- */

    if (upload) {
        mesh->buffer = mVertexBuffers.create(vertices, vCount, indices, iCount);
        if (mesh->buffer == nullptr) {
            NX_INTERNAL_LOG(E, "RENDER: Failed to load mesh; Object pool issue when creating vertex buffer");
            mMeshes.destroy(mesh);
            return nullptr;
        }
    }
    else {
        mesh->buffer = nullptr;
    }

    /* --- Fill mesh data --- */

    mesh->vertices = vertices;
    mesh->indices = indices;

    mesh->vertexCount = vCount;
    mesh->indexCount = iCount;

    mesh->shadowCastMode = NX_SHADOW_CAST_ENABLED;
    mesh->shadowFaceMode = NX_SHADOW_FACE_AUTO;
    mesh->layerMask = NX_LAYER_01;
    mesh->primitiveType = type;
    mesh->aabb = aabb;

    return mesh;
}

inline NX_Mesh* PoolMesh::createMesh(NX_PrimitiveType type, NX_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, bool upload)
{
    SDL_assert(vertices != nullptr && vCount > 0);

    /* --- Calculate the bounding box --- */

    NX_Vec3 min = NX_VEC3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    NX_Vec3 max = NX_VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    if (indices) {
        for (int i = 0; i < iCount; i++) {
            const NX_Vec3& pos = vertices[indices[i]].position;
            min = NX_Vec3Min(min, pos);
            max = NX_Vec3Max(max, pos);
        }
    }
    else {
        for (int i = 0; i < vCount; i++) {
            const NX_Vec3& pos = vertices[i].position;
            min = NX_Vec3Min(min, pos);
            max = NX_Vec3Max(max, pos);
        }
    }

    /* --- Create the mesh --- */

    return createMesh(type, vertices, vCount, indices, iCount, { min, max }, upload);
}

inline void PoolMesh::destroyMesh(NX_Mesh* mesh)
{
    if (mesh != nullptr) {
        SDL_free(mesh->vertices);
        SDL_free(mesh->indices);
        mMeshes.destroy(mesh);
    }
}

inline void PoolMesh::updateMesh(NX_Mesh* mesh)
{
    if (mesh->buffer == nullptr) {
        mesh->buffer = mVertexBuffers.create(mesh->vertices, mesh->vertexCount, mesh->indices, mesh->indexCount);
        if (mesh->buffer == nullptr) {
            NX_INTERNAL_LOG(E, "RENDER: Failed to upload mesh; Object pool issue when creating vertex buffer");
        }
        return;
    }

    mesh->buffer->vbo().upload(0, mesh->vertexCount * sizeof(NX_Vertex3D), mesh->vertices);
    mesh->buffer->ebo().upload(0, mesh->indexCount * sizeof(uint32_t), mesh->indices);
}

} // namespace render

#endif // NX_RENDER_POOL_MESH_HPP
