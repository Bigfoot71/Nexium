/* NX_MeshData.cpp -- Implementation of Nexium's mesh data module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_MeshData.h>
#include <NX/NX_Memory.h>
#include <NX/NX_Log.h>
#include <cstring>
#include <cmath>

// ============================================================================
// PUBLIC API
// ============================================================================

NX_MeshData NX_CreateMeshData(int vertexCount, int indexCount)
{
    NX_MeshData meshData{};

    if (vertexCount <= 0) {
        NX_LOG(E, "RENDER: Invalid vertex count for mesh creation");
        return meshData;
    }

    meshData.vertices = NX_Calloc<NX_Vertex3D>(vertexCount);
    if (meshData.vertices == nullptr) {
        NX_LOG(E, "RENDER: Failed to allocate memory for mesh vertices");
        return meshData;
    }

    meshData.vertexCount = vertexCount;

    if (indexCount > 0) {
        meshData.indices = NX_Calloc<uint32_t>(indexCount);
        if (meshData.indices == nullptr) {
            NX_LOG(E, "RENDER: Failed to allocate memory for mesh indices");
            NX_Free(meshData.vertices);
            meshData.vertices = nullptr;
            meshData.vertexCount = 0;
            return meshData;
        }
        meshData.indexCount = indexCount;
    }

    return meshData;
}

void NX_DestroyMeshData(NX_MeshData* meshData)
{
    if (meshData == nullptr) return;

    NX_Free(meshData->vertices);
    NX_Free(meshData->indices);

    meshData->vertices = nullptr;
    meshData->indices = nullptr;
    meshData->vertexCount = 0;
    meshData->indexCount = 0;
}

NX_MeshData NX_DuplicateMeshData(const NX_MeshData* meshData)
{
    NX_MeshData duplicate{};

    if (meshData == nullptr || meshData->vertices == nullptr) {
        NX_LOG(E, "RENDER: Cannot duplicate null mesh data");
        return duplicate;
    }

    duplicate = NX_CreateMeshData(meshData->vertexCount, meshData->indexCount);

    if (duplicate.vertices == nullptr) {
        return duplicate;
    }

    std::memcpy(duplicate.vertices, meshData->vertices, meshData->vertexCount * sizeof(NX_Vertex3D));

    if (meshData->indexCount > 0 && meshData->indices != nullptr && duplicate.indices != nullptr) {
        std::memcpy(duplicate.indices, meshData->indices, meshData->indexCount * sizeof(uint32_t));
    }

    return duplicate;
}

NX_MeshData NX_MergeMeshData(const NX_MeshData* a, const NX_MeshData* b)
{
    NX_MeshData merged{};

    if (a == nullptr || b == nullptr || a->vertices == nullptr || b->vertices == nullptr) {
        NX_LOG(E, "RENDER: Cannot merge null mesh data");
        return merged;
    }

    int totalVertices = a->vertexCount + b->vertexCount;
    int totalIndices = a->indexCount + b->indexCount;

    merged = NX_CreateMeshData(totalVertices, totalIndices);

    if (merged.vertices == nullptr) {
        return merged;
    }

    std::memcpy(merged.vertices, a->vertices, a->vertexCount * sizeof(NX_Vertex3D));
    std::memcpy(merged.vertices + a->vertexCount, b->vertices, b->vertexCount * sizeof(NX_Vertex3D));

    if (a->indexCount > 0 && a->indices != nullptr) {
        std::memcpy(merged.indices, a->indices, a->indexCount * sizeof(uint32_t));
    }

    if (b->indexCount > 0 && b->indices != nullptr) {
        for (int i = 0; i < b->indexCount; i++) {
            merged.indices[a->indexCount + i] = b->indices[i] + a->vertexCount;
        }
    }

    return merged;
}

void NX_TranslateMeshData(NX_MeshData* meshData, NX_Vec3 translation)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;
    
    for (int i = 0; i < meshData->vertexCount; i++) {
        meshData->vertices[i].position += translation;
    }
}

void NX_RotateMeshData(NX_MeshData* meshData, NX_Quat rotation)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;

    for (int i = 0; i < meshData->vertexCount; i++)
    {
        meshData->vertices[i].position *= rotation;
        meshData->vertices[i].normal *= rotation;

        // Preserve w component for handedness
        NX_Vec3 tangentVec = NX_VEC3(
            meshData->vertices[i].tangent.x, 
            meshData->vertices[i].tangent.y, 
            meshData->vertices[i].tangent.z
        ) * rotation;
        meshData->vertices[i].tangent.x = tangentVec.x;
        meshData->vertices[i].tangent.y = tangentVec.y;
        meshData->vertices[i].tangent.z = tangentVec.z;
    }
}

void NX_ScaleMeshData(NX_MeshData* meshData, NX_Vec3 scale)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;

    for (int i = 0; i < meshData->vertexCount; i++) {
        meshData->vertices[i].position.x *= scale.x;
        meshData->vertices[i].position.y *= scale.y;
        meshData->vertices[i].position.z *= scale.z;
    }

    if (scale.x != scale.y || scale.y != scale.z) {
        NX_GenMeshDataNormals(meshData);
        NX_GenMeshDataTangents(meshData);
    }
}

void NX_GenMeshDataUVsPlanar(NX_MeshData* meshData, NX_Vec2 uvScale, NX_Vec3 axis)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;

    axis = NX_Vec3Normalize(axis);

    NX_Vec3 up = (std::abs(axis.y) < 0.999f) ? NX_VEC3(0, 1, 0) : NX_VEC3(1, 0, 0);
    NX_Vec3 tangent = NX_Vec3Normalize(NX_Vec3Cross(up, axis));
    NX_Vec3 bitangent = NX_Vec3Cross(axis, tangent);
    
    for (int i = 0; i < meshData->vertexCount; i++) {
        NX_Vec3 pos = meshData->vertices[i].position;
        float u = NX_Vec3Dot(pos, tangent) * uvScale.x;
        float v = NX_Vec3Dot(pos, bitangent) * uvScale.y;
        meshData->vertices[i].texcoord = NX_VEC2(u, v);
    }
}

void NX_GenMeshDataUVsSpherical(NX_MeshData* meshData)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;

    for (int i = 0; i < meshData->vertexCount; i++) {
        NX_Vec3 pos = NX_Vec3Normalize(meshData->vertices[i].position);
        float u = 0.5f + std::atan2(pos.z, pos.x) / (2.0f * NX_PI);
        float v = 0.5f - std::asin(pos.y) / NX_PI;
        meshData->vertices[i].texcoord = NX_VEC2(u, v);
    }
}

void NX_GenMeshDataUVsCylindrical(NX_MeshData* meshData)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;

    for (int i = 0; i < meshData->vertexCount; i++) {
        NX_Vec3 pos = meshData->vertices[i].position;
        float u = 0.5f + std::atan2(pos.z, pos.x) / (2.0f * NX_PI);
        float v = pos.y;
        meshData->vertices[i].texcoord = NX_VEC2(u, v);
    }
}

void NX_GenMeshDataNormals(NX_MeshData* meshData)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;

    for (int i = 0; i < meshData->vertexCount; i++) {
        meshData->vertices[i].normal = NX_VEC3(0, 0, 0);
    }

    if (meshData->indexCount > 0 && meshData->indices != nullptr) {
        for (int i = 0; i < meshData->indexCount; i += 3)
        {
            uint32_t i0 = meshData->indices[i];
            uint32_t i1 = meshData->indices[i + 1];
            uint32_t i2 = meshData->indices[i + 2];

            NX_Vec3 v0 = meshData->vertices[i0].position;
            NX_Vec3 v1 = meshData->vertices[i1].position;
            NX_Vec3 v2 = meshData->vertices[i2].position;

            NX_Vec3 edge1 = v1 - v0;
            NX_Vec3 edge2 = v2 - v0;
            NX_Vec3 faceNormal = NX_Vec3Cross(edge1, edge2);

            meshData->vertices[i0].normal += faceNormal;
            meshData->vertices[i1].normal += faceNormal;
            meshData->vertices[i2].normal += faceNormal;
        }
    }
    else {
        for (int i = 0; i < meshData->vertexCount; i += 3)
        {
            NX_Vec3 v0 = meshData->vertices[i].position;
            NX_Vec3 v1 = meshData->vertices[i + 1].position;
            NX_Vec3 v2 = meshData->vertices[i + 2].position;

            NX_Vec3 edge1 = v1 - v0;
            NX_Vec3 edge2 = v2 - v0;
            NX_Vec3 faceNormal = NX_Vec3Cross(edge1, edge2);

            meshData->vertices[i].normal += faceNormal;
            meshData->vertices[i + 1].normal += faceNormal;
            meshData->vertices[i + 2].normal += faceNormal;
        }
    }

    for (int i = 0; i < meshData->vertexCount; i++) {
        meshData->vertices[i].normal = NX_Vec3Normalize(meshData->vertices[i].normal);
    }
}

void NX_GenMeshDataTangents(NX_MeshData* meshData)
{
    if (meshData == nullptr || meshData->vertices == nullptr) return;

    NX_Vec3* bitangents = NX_Calloc<NX_Vec3>(meshData->vertexCount);
    if (bitangents == nullptr) {
        NX_LOG(E, "RENDER: Failed to allocate memory for tangent calculation");
        return;
    }

    for (int i = 0; i < meshData->vertexCount; i++) {
        meshData->vertices[i].tangent = NX_VEC4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    auto processTriangle = [&](uint32_t i0, uint32_t i1, uint32_t i2)
    {
        const NX_Vec3& v0 = meshData->vertices[i0].position;
        const NX_Vec3& v1 = meshData->vertices[i1].position;
        const NX_Vec3& v2 = meshData->vertices[i2].position;

        const NX_Vec2& uv0 = meshData->vertices[i0].texcoord;
        const NX_Vec2& uv1 = meshData->vertices[i1].texcoord;
        const NX_Vec2& uv2 = meshData->vertices[i2].texcoord;

        NX_Vec3 edge1 = v1 - v0;
        NX_Vec3 edge2 = v2 - v0;

        NX_Vec2 deltaUV1 = uv1 - uv0;
        NX_Vec2 deltaUV2 = uv2 - uv0;

        float det = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;

        // Skip the degenerate cases (collinear UVs)
        if (std::abs(det) < 1e-6f) {
            return;
        }

        float invDet = 1.0f / det;

        NX_Vec3 tangent = {
            invDet * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
            invDet * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
            invDet * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
        };

        NX_Vec3 bitangent = {
            invDet * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
            invDet * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
            invDet * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z)
        };

        meshData->vertices[i0].tangent.x += tangent.x;
        meshData->vertices[i0].tangent.y += tangent.y;
        meshData->vertices[i0].tangent.z += tangent.z;

        meshData->vertices[i1].tangent.x += tangent.x;
        meshData->vertices[i1].tangent.y += tangent.y;
        meshData->vertices[i1].tangent.z += tangent.z;

        meshData->vertices[i2].tangent.x += tangent.x;
        meshData->vertices[i2].tangent.y += tangent.y;
        meshData->vertices[i2].tangent.z += tangent.z;

        bitangents[i0] += bitangent;
        bitangents[i1] += bitangent;
        bitangents[i2] += bitangent;
    };

    if (meshData->indexCount > 0 && meshData->indices != nullptr) {
        for (int i = 0; i < meshData->indexCount; i += 3) {
            processTriangle(
                meshData->indices[i],
                meshData->indices[i + 1],
                meshData->indices[i + 2]
            );
        }
    }
    else {
        for (int i = 0; i < meshData->vertexCount; i += 3) {
            processTriangle(i, i + 1, i + 2);
        }
    }

    // Orthogonalization (Gram-Schmidt) and handedness calculation
    for (int i = 0; i < meshData->vertexCount; i++)
    {
        const NX_Vec3& n = meshData->vertices[i].normal;
        NX_Vec3 t = {
            meshData->vertices[i].tangent.x,
            meshData->vertices[i].tangent.y,
            meshData->vertices[i].tangent.z
        };

        // Gram-Schmidt orthogonalization
        t = t - n * NX_Vec3Dot(n, t);

        float tLength = NX_Vec3Length(t);
        if (tLength > 1e-6f) {
            t = t * (1.0f / tLength);
        }
        else {
            // Fallback: generate an arbitrary tangent perpendicular to the normal
            t = std::abs(n.x) < 0.9f ? NX_VEC3_RIGHT : NX_VEC3_UP;
            t = NX_Vec3Normalize(t - n * NX_Vec3Dot(n, t));
        }

        float handedness = (NX_Vec3Dot(NX_Vec3Cross(n, t), bitangents[i]) < 0.0f) ? -1.0f : 1.0f;
        meshData->vertices[i].tangent = NX_VEC4(t.x, t.y, t.z, handedness);
    }

    NX_Free(bitangents);
}

NX_BoundingBox3D NX_CalculateMeshDataAABB(const NX_MeshData* meshData)
{
    NX_BoundingBox3D bounds{};

    if (meshData == nullptr || meshData->vertices == nullptr || meshData->vertexCount == 0) {
        return bounds;
    }

    bounds.min = meshData->vertices[0].position;
    bounds.max = meshData->vertices[0].position;

    for (int i = 1; i < meshData->vertexCount; i++) {
        NX_Vec3 pos = meshData->vertices[i].position;
        bounds.min = NX_Vec3Min(bounds.min, pos);
        bounds.max = NX_Vec3Max(bounds.max, pos);
    }

    return bounds;
}
