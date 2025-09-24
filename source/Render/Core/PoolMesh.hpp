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

#ifndef HP_RENDER_POOL_MESH_HPP
#define HP_RENDER_POOL_MESH_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../HP_VertexBuffer.hpp"
#include "Hyperion/HP_Render.h"
#include <cfloat>

namespace render {

/* === Declaration === */

class PoolMesh {
public:
    HP_Mesh* create(HP_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, const HP_BoundingBox& aabb, bool upload);
    HP_Mesh* create(HP_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, bool upload);
    void destroy(HP_Mesh* mesh);
    void upload(HP_Mesh* mesh);

private:
    util::ObjectPool<HP_VertexBuffer, 512> mVertexBuffers{};
    util::ObjectPool<HP_Mesh, 512> mMeshes{};
};

/* === Public Implementation === */

inline HP_Mesh* PoolMesh::create(HP_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, const HP_BoundingBox& aabb, bool upload)
{
    SDL_assert(vertices != nullptr && vCount > 0);

    /* --- Reserve the object in the pool --- */

    HP_Mesh* mesh = mMeshes.create();
    if (mesh == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load mesh; Object pool issue");
        return nullptr;
    }

    /* --- Creating the GPU vertex buffer --- */

    if (upload) {
        mesh->buffer = mVertexBuffers.create(vertices, vCount, indices, iCount);
        if (mesh->buffer == nullptr) {
            HP_INTERNAL_LOG(E, "RENDER: Failed to load mesh; Object pool issue when creating vertex buffer");
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

    mesh->aabb = aabb;

    mesh->layerMask = HP_LAYER_01;

    return mesh;
}

inline HP_Mesh* PoolMesh::create(HP_Vertex3D* vertices, int vCount, uint32_t* indices, int iCount, bool upload)
{
    SDL_assert(vertices != nullptr && vCount > 0);

    /* --- Calculate the bounding box --- */

    HP_Vec3 min = HP_VEC3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    HP_Vec3 max = HP_VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    if (indices) {
        for (int i = 0; i < iCount; i++) {
            const HP_Vec3& pos = vertices[indices[i]].position;
            min = HP_Vec3Min(min, pos);
            max = HP_Vec3Max(max, pos);
        }
    }
    else {
        for (int i = 0; i < vCount; i++) {
            const HP_Vec3& pos = vertices[i].position;
            min = HP_Vec3Min(min, pos);
            max = HP_Vec3Max(max, pos);
        }
    }

    /* --- Create the mesh --- */

    return create(vertices, vCount, indices, iCount, { min, max }, upload);
}

inline void PoolMesh::destroy(HP_Mesh* mesh)
{
    if (mesh != nullptr) {
        SDL_free(mesh->vertices);
        SDL_free(mesh->indices);
        mMeshes.destroy(mesh);
    }
}

inline void PoolMesh::upload(HP_Mesh* mesh)
{
    if (mesh->buffer == nullptr) {
        mesh->buffer = mVertexBuffers.create(mesh->vertices, mesh->vertexCount, mesh->indices, mesh->indexCount);
        if (mesh->buffer == nullptr) {
            HP_INTERNAL_LOG(E, "RENDER: Failed to upload mesh; Object pool issue when creating vertex buffer");
        }
        return;
    }

    mesh->buffer->vbo().upload(0, mesh->vertexCount * sizeof(HP_Vertex3D), mesh->vertices);
    mesh->buffer->ebo().upload(0, mesh->indexCount * sizeof(uint32_t), mesh->indices);
}

} // namespace render

#endif // HP_RENDER_POOL_MESH_HPP
