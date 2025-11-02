/* NX_Render.cpp -- API definition for Nexium's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Filesystem.h>
#include <NX/NX_Codepoint.h>
#include <NX/NX_Render2D.h>
#include <NX/NX_Texture.h>
#include <NX/NX_Render.h>
#include <NX/NX_Image.h>
#include <NX/NX_Font.h>
#include <NX/NX_Math.h>
#include <NX/NX_Log.h>

#include "./NX_RenderTexture.hpp"
#include "./NX_Texture.hpp"
#include "./NX_Font.hpp"

#include "./Render/NX_RenderState.hpp"
#include "./NX_InstanceBuffer.hpp"
#include "./INX_GlobalAssets.hpp"
#include "./Detail/Helper.hpp"

/* === Draw3D - Public API === */

void NX_Begin3D(const NX_Camera* camera, const NX_Environment* env, const NX_RenderTexture* target)
{
    gRender->scene.begin(
        camera ? *camera : NX_GetDefaultCamera(),
        env ? *env : NX_GetDefaultEnvironment(),
        target
    );
}

void NX_End3D()
{
    gRender->scene.end();
}

void NX_DrawMesh3D(const NX_Mesh* mesh, const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        mesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawMeshInstanced3D(const NX_Mesh* mesh, const NX_InstanceBuffer* instances, int instanceCount,
                            const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        mesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMesh3D(const NX_DynamicMesh* dynMesh, const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        dynMesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMeshInstanced3D(const NX_DynamicMesh* dynMesh, const NX_InstanceBuffer* instances, int instanceCount,
                                   const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        dynMesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModel3D(const NX_Model* model, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        *model, nullptr, 0,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModelInstanced3D(const NX_Model* model, const NX_InstanceBuffer* instances, int instanceCount, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        *model, instances, instanceCount,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

/* === Mesh - Public API === */

NX_Mesh* NX_CreateMesh(NX_PrimitiveType type, const NX_Vertex3D* vertices, int vertexCount, const uint32_t* indices, int indexCount)
{
    /* --- Validation of parameters --- */

    if (vertices == nullptr || vertexCount == 0) {
        NX_LOG(E, "RENDER: Failed to create mesh; Vertices and their count cannot be null");
        return nullptr;
    }

    /* --- Copies of input data --- */

    NX_Vertex3D* vCopy = NX_Malloc<NX_Vertex3D>(vertexCount);
    SDL_memcpy(vCopy, vertices, vertexCount * sizeof(NX_Vertex3D));

    uint32_t* iCopy = nullptr;
    if (indices != nullptr && indexCount > 0) {
        iCopy = NX_Malloc<uint32_t>(indexCount);
        SDL_memcpy(iCopy, indices, indexCount * sizeof(uint32_t));
    }

    /* --- Mesh creation --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        type, vCopy, vertexCount,
        iCopy, indexCount,
        true
    );

    if (mesh == nullptr) {
        NX_Free(vCopy);
        NX_Free(iCopy);
        return nullptr;
    }

    return mesh;
}

NX_Mesh* NX_CreateMeshFrom(NX_PrimitiveType type, NX_Vertex3D* vertices, int vertexCount, uint32_t* indices, int indexCount)
{
    if (vertices == nullptr || vertexCount == 0) {
        NX_LOG(E, "RENDER: Failed to vertex mesh; Vertices and their count cannot be null");
        return nullptr;
    }

    return gRender->meshes.createMesh(
        type, vertices, vertexCount,
        indices, indexCount,
        true
    );
}

void NX_DestroyMesh(NX_Mesh* mesh)
{
    gRender->meshes.destroyMesh(mesh);
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

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    return mesh;
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

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    return mesh;
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

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    return mesh;
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

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    return mesh;
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

    /* --- Mesh creation and finalization --- */

    NX_Mesh* mesh = gRender->meshes.createMesh(
        NX_PRIMITIVE_TRIANGLES,
        vertices, vertexCount,
        indices, indexCount,
        true
    );

    if (mesh == nullptr) {
        NX_Free(vertices);
        NX_Free(indices);
        return nullptr;
    }

    return mesh;
}

void NX_UpdateMeshBuffer(NX_Mesh* mesh)
{
    gRender->meshes.updateMesh(mesh);
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

/* === DnyamicMesh - Public API === */

NX_DynamicMesh* NX_CreateDynamicMesh(size_t initialCapacity)
{
    return gRender->meshes.createDynamicMesh(initialCapacity);
}

void NX_DestroyDynamicMesh(NX_DynamicMesh* dynMesh)
{
    gRender->meshes.destroyDynamicMesh(dynMesh);
}

void NX_BeginDynamicMesh(NX_DynamicMesh* dynMesh, NX_PrimitiveType type)
{
    dynMesh->begin(type);
}

void NX_EndDynamicMesh(NX_DynamicMesh* dynMesh)
{
    dynMesh->end();
}

void NX_SetDynamicMeshTexCoord(NX_DynamicMesh* dynMesh, NX_Vec2 texcoord)
{
    dynMesh->setTexCoord(texcoord);
}

void NX_SetDynamicMeshNormal(NX_DynamicMesh* dynMesh, NX_Vec3 normal)
{
    dynMesh->setNormal(normal);
}

void NX_SetDynamicMeshTangent(NX_DynamicMesh* dynMesh, NX_Vec4 tangent)
{
    dynMesh->setTangent(tangent);
}

void NX_SetDynamicMeshColor(NX_DynamicMesh* dynMesh, NX_Color color)
{
    dynMesh->setColor(color);
}

void NX_AddDynamicMeshVertex(NX_DynamicMesh* dynMesh, NX_Vec3 position)
{
    dynMesh->addVertex(position);
}

void NX_SetDynamicMeshShadowCastMode(NX_DynamicMesh* dynMesh, NX_ShadowCastMode mode)
{
    dynMesh->shadowCastMode = mode;
}

void NX_SetDynamicMeshShadowFaceMode(NX_DynamicMesh* dynMesh, NX_ShadowFaceMode mode)
{
    dynMesh->shadowFaceMode = mode;
}

void NX_SetDynamicMeshLayerMask(NX_DynamicMesh* dynMesh, NX_Layer mask)
{
    dynMesh->layerMask = mask;
}

/* === Model - Public API === */

NX_Model* NX_LoadModel(const char* filePath)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    if (fileData == nullptr || fileSize == 0) {
        NX_LOG(E, "RENDER: Failed to load model data: %s", filePath);
        return nullptr;
    }

    NX_Model* model = gRender->models.loadModel(fileData, fileSize, helper::GetFileExt(filePath));
    NX_Free(fileData);

    return model;
}

NX_Model* NX_LoadModelFromDataory(const void* data, size_t size, const char* hint)
{
    return gRender->models.loadModel(data, size, hint);
}

void NX_DestroyModel(NX_Model* model)
{
    gRender->models.destroyModel(model);
}

void NX_UpdateModelAABB(NX_Model* model, bool updateMeshAABBs)
{
    if (!model || !model->meshes) {
        return;
    }

    NX_Vec3 min = { +FLT_MAX, +FLT_MAX, +FLT_MAX };
    NX_Vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (uint32_t i = 0; i < model->meshCount; i++) {
        NX_Mesh* mesh = model->meshes[i];
        if (updateMeshAABBs) {
            NX_UpdateMeshAABB(mesh);
        }
        min = NX_Vec3Min(min, mesh->aabb.min);
        max = NX_Vec3Max(max, mesh->aabb.max);
    }

    model->aabb.min = min;
    model->aabb.max = max;
}

void NX_ScaleModelAABB(NX_Model* model, float scale, bool scaleMeshAABBs)
{
    if (scaleMeshAABBs) {
        for (int i = 0; i < model->meshCount; i++) {
            model->meshes[i]->aabb.min *= scale;
            model->meshes[i]->aabb.max *= scale;
        }
    }

    model->aabb.min *= scale;
    model->aabb.max *= scale;
}

NX_ModelAnimation** NX_LoadModelAnimations(const char* filePath, int* animCount, int targetFrameRate)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    NX_ModelAnimation** animations = gRender->models.loadAnimations(fileData, fileSize, helper::GetFileExt(filePath), animCount, targetFrameRate);
    NX_Free(fileData);
    return animations;
}

NX_ModelAnimation** NX_LoadModelAnimationsFromDataory(const void* data, unsigned int size, const char* hint, int* animCount, int targetFrameRate)
{
    return gRender->models.loadAnimations(data, size, hint, animCount, targetFrameRate);
}

void NX_DestroyModelAnimations(NX_ModelAnimation** animations, int animCount)
{
    gRender->models.destroyAnimations(animations, animCount);
}

NX_ModelAnimation* NX_GetModelAnimation(NX_ModelAnimation** animations, int animCount, const char* name)
{
    for (int i = 0; i < animCount; i++) {
        if (SDL_strcmp(animations[i]->name, name) == 0) {
            return animations[i];
        }
    }
    return nullptr;
}

/* === Light - Public API === */

NX_Light* NX_CreateLight(NX_LightType type)
{
    return gRender->scene.lights().create(type);
}

void NX_DestroyLight(NX_Light* light)
{
    gRender->scene.lights().destroy(light);
}

bool NX_IsLightActive(const NX_Light* light)
{
    return light->isActive();
}

void NX_SetLightActive(NX_Light* light, bool active)
{
    light->setActive(active);
}

NX_Layer NX_GetLightLayerMask(const NX_Light* light)
{
    return light->layerMask();
}

void NX_SetLightLayerMask(NX_Light* light, NX_Layer layers)
{
    light->setLayerMask(layers);
}

NX_Layer NX_GetLightCullMask(const NX_Light* light)
{
    return light->cullMask();
}

void NX_SetLightCullMask(NX_Light* light, NX_Layer layers)
{
    light->setCullMask(layers);
}

NX_Vec3 NX_GetLightPosition(const NX_Light* light)
{
    return light->position();
}

void NX_SetLightPosition(NX_Light* light, NX_Vec3 position)
{
    light->setPosition(position);
}

NX_Vec3 NX_GetLightDirection(const NX_Light* light)
{
    return light->position();
}

void NX_SetLightDirection(NX_Light* light, NX_Vec3 direction)
{
    light->setDirection(direction);
}

NX_Color NX_GetLightColor(const NX_Light* light)
{
    return light->color();
}

void NX_SetLightColor(NX_Light* light, NX_Color color)
{
    light->setColor(color);
}

float NX_GetLightEnergy(const NX_Light* light)
{
    return light->energy();
}

void NX_SetLightEnergy(NX_Light* light, float energy)
{
    light->setEnergy(energy);
}

float NX_GetLightSpecular(const NX_Light* light)
{
    return light->specular();
}

void NX_SetLightSpecular(NX_Light* light, float specular)
{
    light->setSpecular(specular);
}

float NX_GetLightRange(const NX_Light* light)
{
    return light->range();
}

void NX_SetLightRange(NX_Light* light, float range)
{
    light->setRange(range);
}

float NX_GetLightAttenuation(const NX_Light* light)
{
    return light->energy();
}

void NX_SetLightAttenuation(NX_Light* light, float attenuation)
{
    light->setAttenuation(attenuation);
}

float NX_GetLightInnerCutOff(const NX_Light* light)
{
    return light->innerCutOff();
}

void NX_SetLightInnerCutOff(NX_Light* light, float radians)
{
    light->setInnerCutOff(radians);
}

float NX_GetLightOuterCutOff(const NX_Light* light)
{
    return light->outerCutOff();
}

void NX_SetLightOuterCutOff(NX_Light* light, float radians)
{
    light->setOuterCutOff(radians);
}

void NX_SetLightCutOff(NX_Light* light, float inner, float outer)
{
    light->setInnerCutOff(inner);
    light->setOuterCutOff(outer);
}

bool NX_IsShadowActive(const NX_Light* light)
{
    return light->isShadowActive();
}

void NX_SetShadowActive(NX_Light* light, bool active)
{
    light->setShadowActive(active);
}

NX_Layer NX_GetShadowCullMask(const NX_Light* light)
{
    return light->shadowCullMask();
}

void NX_SetShadowCullMask(NX_Light* light, NX_Layer layers)
{
    light->setShadowCullMask(layers);
}

NXAPI float NX_GetShadowSlopeBias(NX_Light* light)
{
    return light->shadowSlopeBias();
}

NXAPI void NX_SetShadowSlopeBias(NX_Light* light, float slopeBias)
{
    light->setShadowSlopeBias(slopeBias);
}

NXAPI float NX_GetShadowBias(NX_Light* light)
{
    return light->shadowBias();
}

NXAPI void NX_SetShadowBias(NX_Light* light, float bias)
{
    light->setShadowBias(bias);
}

float NX_GetShadowSoftness(const NX_Light* light)
{
    return light->shadowSoftness();
}

void NX_SetShadowSoftness(NX_Light* light, float softness)
{
    light->setShadowSoftness(softness);
}

NX_ShadowUpdateMode NX_GetShadowUpdateMode(const NX_Light* light)
{
    return light->shadowUpdateMode();
}

void NX_SetShadowUpdateMode(NX_Light* light, NX_ShadowUpdateMode mode)
{
    light->setShadowUpdateMode(mode);
}

float NX_GetShadowUpdateInterval(const NX_Light* light)
{
    return light->shadowUpdateInterval();
}

void NX_SetShadowUpdateInterval(NX_Light* light, float sec)
{
    light->setShadowUpdateInterval(sec);
}

void NX_UpdateShadowMap(NX_Light* light)
{
    light->forceShadowMapUpdate();
}
