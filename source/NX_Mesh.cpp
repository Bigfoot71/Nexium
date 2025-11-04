/* NX_Mesh.cpp -- API definition for Nexium's mesh module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Mesh.h>

#include "./INX_GlobalPool.hpp"
#include "./NX_Vertex.hpp"
#include "NX/NX_Shape.h"

#include <SDL3/SDL_stdinc.h>
#include <NX/NX_Memory.h>
#include <NX/NX_Log.h>
#include <cfloat>

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Mesh* NX_CreateMesh(NX_PrimitiveType type, const NX_Vertex3D* vertices, int vertexCount, const uint32_t* indices, int indexCount, const NX_BoundingBox3D* aabb)
{
    if (vertices == nullptr || vertexCount == 0) {
        NX_LOG(E, "RENDER: Failed to create mesh; Vertices and their count cannot be null");
        return nullptr;
    }

    NX_Vertex3D* vCopy = NX_Malloc<NX_Vertex3D>(vertexCount);
    SDL_memcpy(vCopy, vertices, vertexCount * sizeof(NX_Vertex3D));

    uint32_t* iCopy = nullptr;
    if (indices != nullptr && indexCount > 0) {
        iCopy = NX_Malloc<uint32_t>(indexCount);
        SDL_memcpy(iCopy, indices, indexCount * sizeof(uint32_t));
    }

    NX_Mesh* mesh = NX_CreateMeshFrom(type, vCopy, vertexCount, iCopy, indexCount, aabb);
    if (mesh == nullptr) {
        NX_Free(vCopy);
        NX_Free(iCopy);
        return nullptr;
    }

    return mesh;
}

NX_Mesh* NX_CreateMeshFrom(NX_PrimitiveType type, NX_Vertex3D* vertices, int vertexCount, uint32_t* indices, int indexCount, const NX_BoundingBox3D* aabb)
{
    if (vertices == nullptr || vertexCount == 0) {
        NX_LOG(E, "RENDER: Failed to vertex mesh; Vertices and their count cannot be null");
        return nullptr;
    }

    NX_Mesh* mesh = INX_Pool.Create<NX_Mesh>();
    if (mesh == nullptr) {
        return nullptr;
    }

    mesh->buffer = INX_Pool.Create<NX_VertexBuffer3D>(vertices, vertexCount, indices, indexCount);
    mesh->vertices = vertices;
    mesh->indices = indices;

    mesh->vertexCount = vertexCount;
    mesh->indexCount = indexCount;
    mesh->primitiveType = type;

    mesh->shadowCastMode = NX_SHADOW_CAST_ENABLED;
    mesh->shadowFaceMode = NX_SHADOW_FACE_AUTO;
    mesh->layerMask = NX_LAYER_01;

    if (aabb != nullptr) mesh->aabb = *aabb;
    else NX_UpdateMeshAABB(mesh);

    return mesh;
}

void NX_DestroyMesh(NX_Mesh* mesh)
{
    INX_Pool.Destroy(mesh->buffer);
    INX_Pool.Destroy(mesh);
}

NX_Mesh* NX_GenMeshQuad(NX_Vec2 size, NX_IVec2 subDiv, NX_Vec3 normal)
{
    /* --- Parameter validation --- */

    size.x = std::max(0.1f, size.x);
    size.y = std::max(0.1f, size.y);
    int segX = std::max(1, subDiv.x);
    int segY = std::max(1, subDiv.y);

    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length < 0.001f) {
        normal = NX_VEC3(0.0f, 0.0f, 1.0f);
        length = 1.0f;
    }
    normal.x /= length;
    normal.y /= length;
    normal.z /= length;

    /* --- Memory allocation --- */

    int vertexCount = (segX + 1) * (segY + 1);
    int indexCount = segX * segY * 6;

    NX_Vertex3D* vertices = NX_Malloc<NX_Vertex3D>(vertexCount);
    uint32_t* indices = NX_Malloc<uint32_t>(indexCount);

    if (!vertices || !indices) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    /* --- Orientation vectors --- */

    NX_Vec3 reference = (std::abs(normal.y) < 0.9f) ? NX_VEC3(0.0f, 1.0f, 0.0f) : NX_VEC3(1.0f, 0.0f, 0.0f);

    NX_Vec3 tangent;
    tangent.x = normal.y * reference.z - normal.z * reference.y;
    tangent.y = normal.z * reference.x - normal.x * reference.z;
    tangent.z = normal.x * reference.y - normal.y * reference.x;

    float tangentLength = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
    tangent.x /= tangentLength;
    tangent.y /= tangentLength;
    tangent.z /= tangentLength;

    NX_Vec3 bitangent;
    bitangent.x = normal.y * tangent.z - normal.z * tangent.y;
    bitangent.y = normal.z * tangent.x - normal.x * tangent.z;
    bitangent.z = normal.x * tangent.y - normal.y * tangent.x;

    /* --- Vertex generation --- */

    int vertexIndex = 0;
    for (int y = 0; y <= segY; y++) {
        for (int x = 0; x <= segX; x++) {
            NX_Vertex3D& vertex = vertices[vertexIndex++];

            float u = ((float)x / segX) - 0.5f;
            float v = ((float)y / segY) - 0.5f;
            float localX = u * size.x;
            float localY = v * size.y;

            vertex.position.x = localX * tangent.x + localY * bitangent.x;
            vertex.position.y = localX * tangent.y + localY * bitangent.y;
            vertex.position.z = localX * tangent.z + localY * bitangent.z;

            vertex.texcoord.x = (float)x / segX;
            vertex.texcoord.y = (float)y / segY;
            vertex.normal = normal;
            vertex.tangent = NX_VEC4(tangent.x, tangent.y, tangent.z, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    /* --- Index generation --- */

    int indexIndex = 0;
    for (int y = 0; y < segY; y++) {
        for (int x = 0; x < segX; x++) {
            uint32_t i0 = y * (segX + 1) + x;
            uint32_t i1 = y * (segX + 1) + (x + 1);
            uint32_t i2 = (y + 1) * (segX + 1) + (x + 1);
            uint32_t i3 = (y + 1) * (segX + 1) + x;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Create and return the mesh --- */

    return NX_CreateMeshFrom(NX_PRIMITIVE_TRIANGLES, vertices, vertexCount, indices, indexCount, nullptr);
}

NX_Mesh* NX_GenMeshCube(NX_Vec3 size, NX_IVec3 subDiv)
{
    /* --- Parameter validation --- */

    int segX = std::max(1, subDiv.x);
    int segY = std::max(1, subDiv.y);
    int segZ = std::max(1, subDiv.z);

    /* --- Memory allocation --- */

    int verticesFrontBack = (segX + 1) * (segY + 1);
    int verticesLeftRight = (segZ + 1) * (segY + 1);
    int verticesTopBottom = (segX + 1) * (segZ + 1);
    int vertexCount = 2 * (verticesFrontBack + verticesLeftRight + verticesTopBottom);

    int indicesFrontBack = segX * segY * 6;
    int indicesLeftRight = segZ * segY * 6;
    int indicesTopBottom = segX * segZ * 6;
    int indexCount = 2 * (indicesFrontBack + indicesLeftRight + indicesTopBottom);

    NX_Vertex3D* vertices = NX_Malloc<NX_Vertex3D>(vertexCount);
    uint32_t* indices = NX_Malloc<uint32_t>(indexCount);

    if (!vertices || !indices) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    /* --- Face configuration --- */

    struct FaceParams {
        NX_Vec3 normal;
        NX_Vec4 tangent;
        int segsU, segsV;
    };

    FaceParams faces[6] = {
        {NX_VEC3(0, 0, 1), NX_VEC4(1, 0, 0, 1), segX, segY},   // Front (Z+)
        {NX_VEC3(0, 0, -1), NX_VEC4(-1, 0, 0, 1), segX, segY}, // Back (Z-)
        {NX_VEC3(1, 0, 0), NX_VEC4(0, 0, -1, 1), segZ, segY},  // Right (X+)
        {NX_VEC3(-1, 0, 0), NX_VEC4(0, 0, 1, 1), segZ, segY},  // Left (X-)
        {NX_VEC3(0, 1, 0), NX_VEC4(1, 0, 0, 1), segX, segZ},   // Top (Y+)
        {NX_VEC3(0, -1, 0), NX_VEC4(1, 0, 0, 1), segX, segZ}   // Bottom (Y-)
    };

    /* --- Vertex and index generation --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    NX_Vec3 halfSize = size * 0.5f;

    for (int face = 0; face < 6; face++) {
        uint32_t baseVertex = vertexIndex;
        FaceParams& fp = faces[face];

        for (int v = 0; v <= fp.segsV; v++) {
            for (int u = 0; u <= fp.segsU; u++) {
                NX_Vertex3D& vertex = vertices[vertexIndex++];

                float uNorm = (float)u / fp.segsU;
                float vNorm = (float)v / fp.segsV;

                switch (face) {
                    case 0: // Front (Z+)
                        vertex.position.x = -halfSize.x + size.x * uNorm;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = halfSize.z;
                        break;
                    case 1: // Back (Z-)
                        vertex.position.x = halfSize.x - size.x * uNorm;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = -halfSize.z;
                        break;
                    case 2: // Right (X+)
                        vertex.position.x = halfSize.x;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = halfSize.z - size.z * uNorm;
                        break;
                    case 3: // Left (X-)
                        vertex.position.x = -halfSize.x;
                        vertex.position.y = -halfSize.y + size.y * vNorm;
                        vertex.position.z = -halfSize.z + size.z * uNorm;
                        break;
                    case 4: // Top (Y+)
                        vertex.position.x = -halfSize.x + size.x * uNorm;
                        vertex.position.y = halfSize.y;
                        vertex.position.z = halfSize.z - size.z * vNorm;
                        break;
                    case 5: // Bottom (Y-)
                        vertex.position.x = -halfSize.x + size.x * uNorm;
                        vertex.position.y = -halfSize.y;
                        vertex.position.z = -halfSize.z + size.z * vNorm;
                        break;
                }

                vertex.texcoord.x = uNorm;
                vertex.texcoord.y = vNorm;
                vertex.normal = fp.normal;
                vertex.tangent = fp.tangent;
                vertex.color = NX_COLOR(1, 1, 1, 1);
            }
        }

        // Generate indices for this face
        for (int v = 0; v < fp.segsV; v++) {
            for (int u = 0; u < fp.segsU; u++) {
                uint32_t i0 = baseVertex + v * (fp.segsU + 1) + u;
                uint32_t i1 = baseVertex + v * (fp.segsU + 1) + (u + 1);
                uint32_t i2 = baseVertex + (v + 1) * (fp.segsU + 1) + (u + 1);
                uint32_t i3 = baseVertex + (v + 1) * (fp.segsU + 1) + u;

                indices[indexIndex++] = i0;
                indices[indexIndex++] = i1;
                indices[indexIndex++] = i2;

                indices[indexIndex++] = i0;
                indices[indexIndex++] = i2;
                indices[indexIndex++] = i3;
            }
        }
    }

    /* --- Create and return the mesh --- */

    return NX_CreateMeshFrom(NX_PRIMITIVE_TRIANGLES, vertices, vertexCount, indices, indexCount, nullptr);
}

NX_Mesh* NX_GenMeshSphere(float radius, int slices, int rings)
{
    /* --- Parameter validation --- */

    radius = std::max(0.1f, radius);
    slices = std::max(3, slices);
    rings = std::max(2, rings);

    /* --- Memory allocation --- */

    int vertexCount = (rings + 1) * (slices + 1);
    int indexCount = rings * slices * 6;

    NX_Vertex3D* vertices = NX_Malloc<NX_Vertex3D>(vertexCount);
    uint32_t* indices = NX_Malloc<uint32_t>(indexCount);

    if (!vertices || !indices) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    /* --- Sphere generation --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float piOverRings = NX_PI / rings;
    const float tauOverSlices = NX_TAU / slices;

    for (int ring = 0; ring <= rings; ring++) {
        float phi = ring * piOverRings;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        float y = radius * cosPhi;
        float ringRadius = radius * sinPhi;

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * tauOverSlices;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal = NX_VEC3(
                vertex.position.x / radius,
                vertex.position.y / radius,
                vertex.position.z / radius
            );

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = (float)ring / rings;
            vertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    /* --- Index generation --- */

    for (int ring = 0; ring < rings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t current = ring * (slices + 1) + slice;
            uint32_t next = current + slices + 1;

            uint32_t i0 = current;
            uint32_t i1 = current + 1;
            uint32_t i2 = next + 1;
            uint32_t i3 = next;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Create and return the mesh --- */

    return NX_CreateMeshFrom(NX_PRIMITIVE_TRIANGLES, vertices, vertexCount, indices, indexCount, nullptr);
}

NX_Mesh* NX_GenMeshCylinder(float topRadius, float bottomRadius, float height, int slices, int rings, bool topCap, bool bottomCap)
{
    /* --- Parameter validation --- */

    topRadius = std::max(0.0f, topRadius);
    bottomRadius = std::max(0.0f, bottomRadius);
    height = std::max(0.1f, height);
    slices = std::max(3, slices);
    rings = std::max(1, rings);

    if (topRadius == 0.0f && bottomRadius == 0.0f) {
        bottomRadius = 1.0f;
    }

    /* --- Memory allocation --- */

    int sideVertices = (rings + 1) * (slices + 1);
    int topCapVertices = (topCap && topRadius > 0.0f) ? slices + 2 : 0;
    int bottomCapVertices = (bottomCap && bottomRadius > 0.0f) ? slices + 2 : 0;
    int vertexCount = sideVertices + topCapVertices + bottomCapVertices;

    int sideIndices = rings * slices * 6;
    int topCapIndices = (topCap && topRadius > 0.0f) ? slices * 3 : 0;
    int bottomCapIndices = (bottomCap && bottomRadius > 0.0f) ? slices * 3 : 0;
    int indexCount = sideIndices + topCapIndices + bottomCapIndices;

    NX_Vertex3D* vertices = NX_Malloc<NX_Vertex3D>(vertexCount);
    uint32_t* indices = NX_Malloc<uint32_t>(indexCount);

    if (!vertices || !indices) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    /* --- Cylinder setup --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float angleStep = NX_TAU / slices;
    const float halfHeight = height * 0.5f;

    NX_Vec3 sideNormalBase;
    if (topRadius != bottomRadius) {
        float radiusDiff = bottomRadius - topRadius;
        float normalLength = std::sqrt(radiusDiff * radiusDiff + height * height);
        sideNormalBase = NX_VEC3(radiusDiff / normalLength, height / normalLength, 0.0f);
    }
    else {
        sideNormalBase = NX_VEC3(1.0f, 0.0f, 0.0f);
    }

    /* --- Side generation --- */

    uint32_t sideBaseVertex = vertexIndex;

    for (int ring = 0; ring <= rings; ring++) {
        float t = (float)ring / rings;
        float y = -halfHeight + height * t;
        float currentRadius = bottomRadius + (topRadius - bottomRadius) * t;

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = std::cos(angle);
            float sinAngle = std::sin(angle);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = currentRadius * cosAngle;
            vertex.position.y = y;
            vertex.position.z = currentRadius * sinAngle;

            vertex.normal.x = sideNormalBase.x * cosAngle;
            vertex.normal.y = sideNormalBase.y;
            vertex.normal.z = sideNormalBase.x * sinAngle;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = t;
            vertex.tangent = NX_VEC4(-sinAngle, 0.0f, cosAngle, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    for (int ring = 0; ring < rings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = sideBaseVertex + ring * (slices + 1) + slice;
            uint32_t i1 = sideBaseVertex + ring * (slices + 1) + (slice + 1);
            uint32_t i2 = sideBaseVertex + (ring + 1) * (slices + 1) + (slice + 1);
            uint32_t i3 = sideBaseVertex + (ring + 1) * (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i1;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i3;
            indices[indexIndex++] = i2;
        }
    }

    /* --- Top cap generation --- */

    if (topCap && topRadius > 0.0f) {
        uint32_t topCapBaseVertex = vertexIndex;

        NX_Vertex3D& centerVertex = vertices[vertexIndex++];
        centerVertex.position = NX_VEC3(0.0f, halfHeight, 0.0f);
        centerVertex.normal = NX_VEC3(0.0f, 1.0f, 0.0f);
        centerVertex.texcoord = NX_VEC2(0.5f, 0.5f);
        centerVertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        centerVertex.color = NX_COLOR(1, 1, 1, 1);

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = std::cos(angle);
            float sinAngle = std::sin(angle);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = topRadius * cosAngle;
            vertex.position.y = halfHeight;
            vertex.position.z = topRadius * sinAngle;
            vertex.normal = NX_VEC3(0.0f, 1.0f, 0.0f);
            vertex.texcoord.x = 0.5f + 0.5f * cosAngle;
            vertex.texcoord.y = 0.5f + 0.5f * sinAngle;
            vertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice < slices; slice++) {
            indices[indexIndex++] = topCapBaseVertex;
            indices[indexIndex++] = topCapBaseVertex + 1 + (slice + 1);
            indices[indexIndex++] = topCapBaseVertex + 1 + slice;
        }
    }

    /* --- Bottom cap generation --- */

    if (bottomCap && bottomRadius > 0.0f) {
        uint32_t bottomCapBaseVertex = vertexIndex;

        NX_Vertex3D& centerVertex = vertices[vertexIndex++];
        centerVertex.position = NX_VEC3(0.0f, -halfHeight, 0.0f);
        centerVertex.normal = NX_VEC3(0.0f, -1.0f, 0.0f);
        centerVertex.texcoord = NX_VEC2(0.5f, 0.5f);
        centerVertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        centerVertex.color = NX_COLOR(1, 1, 1, 1);

        for (int slice = 0; slice <= slices; slice++) {
            float angle = slice * angleStep;
            float cosAngle = std::cos(angle);
            float sinAngle = std::sin(angle);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = bottomRadius * cosAngle;
            vertex.position.y = -halfHeight;
            vertex.position.z = bottomRadius * sinAngle;
            vertex.normal = NX_VEC3(0.0f, -1.0f, 0.0f);
            vertex.texcoord.x = 0.5f + 0.5f * cosAngle;
            vertex.texcoord.y = 0.5f + 0.5f * sinAngle;
            vertex.tangent = NX_VEC4(1.0f, 0.0f, 0.0f, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice < slices; slice++) {
            indices[indexIndex++] = bottomCapBaseVertex;
            indices[indexIndex++] = bottomCapBaseVertex + 1 + slice;
            indices[indexIndex++] = bottomCapBaseVertex + 1 + (slice + 1);
        }
    }

    /* --- Create and return the mesh --- */

    return NX_CreateMeshFrom(NX_PRIMITIVE_TRIANGLES, vertices, vertexCount, indices, indexCount, nullptr);
}

NX_Mesh* NX_GenMeshCapsule(float radius, float height, int slices, int rings)
{
    /* --- Parameter validation --- */

    radius = std::max(0.1f, radius);
    height = std::max(0.0f, height);
    slices = NX_MAX(3, slices);
    rings = NX_MAX(2, rings);

    int hemisphereRings = NX_MAX(1, rings / 2);

    /* --- Memory allocation --- */

    int cylinderVertices = (height > 0.0f) ? 2 * (slices + 1) : 0;
    int topHemisphereVertices = (hemisphereRings + 1) * (slices + 1);
    int bottomHemisphereVertices = (hemisphereRings + 1) * (slices + 1);

    if (height == 0.0f) {
        bottomHemisphereVertices -= (slices + 1);
    }

    int vertexCount = cylinderVertices + topHemisphereVertices + bottomHemisphereVertices;

    int cylinderIndices = (height > 0.0f) ? slices * 6 : 0;
    int hemisphereIndices = hemisphereRings * slices * 6;
    int indexCount = cylinderIndices + 2 * hemisphereIndices;

    NX_Vertex3D* vertices = NX_Malloc<NX_Vertex3D>(vertexCount);
    uint32_t* indices = NX_Malloc<uint32_t>(indexCount);

    if (!vertices || !indices) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    /* --- Capsule setup --- */

    int vertexIndex = 0;
    int indexIndex = 0;
    const float PI_OVER_2 = NX_PI * 0.5f;
    const float angleStep = NX_TAU / slices;
    const float halfHeight = height * 0.5f;

    /* --- Top hemisphere generation --- */

    uint32_t topHemisphereBaseVertex = vertexIndex;

    for (int ring = 0; ring <= hemisphereRings; ring++) {
        float phi = (float)ring / hemisphereRings * PI_OVER_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        float y = halfHeight + radius * cosPhi;
        float ringRadius = radius * sinPhi;

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal.x = sinPhi * cosTheta;
            vertex.normal.y = cosPhi;
            vertex.normal.z = sinPhi * sinTheta;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = 0.5f + 0.5f * ((float)ring / hemisphereRings);
            vertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    for (int ring = 0; ring < hemisphereRings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = topHemisphereBaseVertex + ring * (slices + 1) + slice;
            uint32_t i1 = topHemisphereBaseVertex + ring * (slices + 1) + (slice + 1);
            uint32_t i2 = topHemisphereBaseVertex + (ring + 1) * (slices + 1) + (slice + 1);
            uint32_t i3 = topHemisphereBaseVertex + (ring + 1) * (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Cylinder generation --- */

    uint32_t cylinderBaseVertex = vertexIndex;

    if (height > 0.0f) {
        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            NX_Vertex3D& topVertex = vertices[vertexIndex++];
            topVertex.position.x = radius * cosTheta;
            topVertex.position.y = halfHeight;
            topVertex.position.z = radius * sinTheta;
            topVertex.normal.x = cosTheta;
            topVertex.normal.y = 0.0f;
            topVertex.normal.z = sinTheta;
            topVertex.texcoord.x = (float)slice / slices;
            topVertex.texcoord.y = 0.5f;
            topVertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            topVertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            NX_Vertex3D& bottomVertex = vertices[vertexIndex++];
            bottomVertex.position.x = radius * cosTheta;
            bottomVertex.position.y = -halfHeight;
            bottomVertex.position.z = radius * sinTheta;
            bottomVertex.normal.x = cosTheta;
            bottomVertex.normal.y = 0.0f;
            bottomVertex.normal.z = sinTheta;
            bottomVertex.texcoord.x = (float)slice / slices;
            bottomVertex.texcoord.y = 0.5f;
            bottomVertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            bottomVertex.color = NX_COLOR(1, 1, 1, 1);
        }

        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = cylinderBaseVertex + slice;
            uint32_t i1 = cylinderBaseVertex + (slice + 1);
            uint32_t i2 = cylinderBaseVertex + (slices + 1) + (slice + 1);
            uint32_t i3 = cylinderBaseVertex + (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Bottom hemisphere generation --- */

    uint32_t bottomHemisphereBaseVertex = vertexIndex;
    int startRing = (height == 0.0f) ? 1 : 0;

    for (int ring = startRing; ring <= hemisphereRings; ring++) {
        float phi = PI_OVER_2 + (float)ring / hemisphereRings * PI_OVER_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        float y = -halfHeight + radius * cosPhi;
        float ringRadius = radius * sinPhi;

        for (int slice = 0; slice <= slices; slice++) {
            float theta = slice * angleStep;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            NX_Vertex3D& vertex = vertices[vertexIndex++];

            vertex.position.x = ringRadius * cosTheta;
            vertex.position.y = y;
            vertex.position.z = ringRadius * sinTheta;

            vertex.normal.x = sinPhi * cosTheta;
            vertex.normal.y = cosPhi;
            vertex.normal.z = sinPhi * sinTheta;

            vertex.texcoord.x = (float)slice / slices;
            vertex.texcoord.y = 0.5f - 0.5f * ((float)ring / hemisphereRings);
            vertex.tangent = NX_VEC4(-sinTheta, 0.0f, cosTheta, 1.0f);
            vertex.color = NX_COLOR(1, 1, 1, 1);
        }
    }

    int effectiveRings = hemisphereRings - startRing;
    for (int ring = 0; ring < effectiveRings; ring++) {
        for (int slice = 0; slice < slices; slice++) {
            uint32_t i0 = bottomHemisphereBaseVertex + ring * (slices + 1) + slice;
            uint32_t i1 = bottomHemisphereBaseVertex + ring * (slices + 1) + (slice + 1);
            uint32_t i2 = bottomHemisphereBaseVertex + (ring + 1) * (slices + 1) + (slice + 1);
            uint32_t i3 = bottomHemisphereBaseVertex + (ring + 1) * (slices + 1) + slice;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i1;
            indices[indexIndex++] = i2;

            indices[indexIndex++] = i0;
            indices[indexIndex++] = i2;
            indices[indexIndex++] = i3;
        }
    }

    /* --- Create and return the mesh --- */

    return NX_CreateMeshFrom(NX_PRIMITIVE_TRIANGLES, vertices, vertexCount, indices, indexCount, nullptr);
}

void NX_UpdateMeshBuffer(NX_Mesh* mesh)
{
    if (mesh->buffer == nullptr) {
        mesh->buffer = INX_Pool.Create<NX_VertexBuffer3D>(mesh->vertices, mesh->vertexCount, mesh->indices, mesh->indexCount);
        if (mesh->buffer == nullptr) {
            NX_LOG(E, "RENDER: Failed to upload mesh; Object pool issue when creating vertex buffer");
        }
        return;
    }

    mesh->buffer->vbo.Upload(0, mesh->vertexCount * sizeof(NX_Vertex3D), mesh->vertices);
    mesh->buffer->ebo.Upload(0, mesh->indexCount * sizeof(uint32_t), mesh->indices);
}

void NX_UpdateMeshAABB(NX_Mesh* mesh)
{
    if (!mesh || mesh->vertexCount == 0) {
        return;
    }

    const NX_Vertex3D* vertices = mesh->vertices;
    const uint32_t* indices = mesh->indices;

    mesh->aabb.min = NX_VEC3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    mesh->aabb.max = NX_VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    if (indices) {
        for (int i = 0; i < mesh->indexCount; i++) {
            const NX_Vec3& pos = vertices[indices[i]].position;
            mesh->aabb.min = NX_Vec3Min(mesh->aabb.min, pos);
            mesh->aabb.max = NX_Vec3Max(mesh->aabb.max, pos);
        }
    }
    else {
        for (int i = 0; i < mesh->vertexCount; i++) {
            const NX_Vec3& pos = vertices[i].position;
            mesh->aabb.min = NX_Vec3Min(mesh->aabb.min, pos);
            mesh->aabb.max = NX_Vec3Max(mesh->aabb.max, pos);
        }
    }
}
