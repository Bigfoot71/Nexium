/* NX_DynamicMesh.cpp -- API definition for Nexium's dynamic mesh module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_DynamicMesh.h>
#include "./INX_GlobalPool.hpp"
#include <cfloat>

// ============================================================================
// PUBLIC API
// ============================================================================

NX_DynamicMesh* NX_CreateDynamicMesh(size_t initialCapacity)
{
    NX_DynamicMesh* dynMesh = INX_Pool.Create<NX_DynamicMesh>();

    if (!dynMesh->vertices.Reserve(initialCapacity)) {
        NX_LOG(E, "RENDER: Dynamic mesh vertex buffer memory reservation failed (requested: %zu vertices)", initialCapacity);
    }

    dynMesh->buffer = INX_Pool.Create<NX_VertexBuffer3D>(nullptr, initialCapacity * sizeof(NX_Vertex3D), nullptr, 0);

    dynMesh->current = {
        .position = NX_VEC3_ZERO,
        .texcoord = NX_VEC2_ZERO,
        .normal = NX_VEC3_BACK,
        .tangent = NX_VEC4_IDENTITY,
        .color = NX_WHITE,
        .boneIds = NX_IVEC4_ZERO,
        .weights = NX_VEC4_ZERO
    };

    dynMesh->shadowCastMode = NX_SHADOW_CAST_ENABLED;
    dynMesh->shadowFaceMode = NX_SHADOW_FACE_AUTO;
    dynMesh->primitiveType = NX_PRIMITIVE_TRIANGLES;
    dynMesh->aabb = { NX_VEC3_ZERO, NX_VEC3_ZERO };
    dynMesh->layerMask = NX_LAYER_01;

    return dynMesh;
}

void NX_DestroyDynamicMesh(NX_DynamicMesh* dynMesh)
{
    INX_Pool.Destroy(dynMesh->buffer);
    INX_Pool.Destroy(dynMesh);
}

void NX_BeginDynamicMesh(NX_DynamicMesh* dynMesh, NX_PrimitiveType type)
{
    dynMesh->primitiveType = type;
    dynMesh->vertices.Clear();
    dynMesh->current = {
        .position = NX_VEC3_ZERO,
        .texcoord = NX_VEC2_ZERO,
        .normal = NX_VEC3_BACK,
        .tangent = NX_VEC4_IDENTITY,
        .color = NX_WHITE,
        .boneIds = NX_IVEC4_ZERO,
        .weights = NX_VEC4_ZERO
    };
}

void NX_EndDynamicMesh(NX_DynamicMesh* dynMesh)
{
    dynMesh->buffer->vbo.Reserve(dynMesh->vertices.GetSize() * sizeof(NX_Vertex3D), false);
    dynMesh->buffer->vbo.Upload(0, dynMesh->vertices.GetSize() * sizeof(NX_Vertex3D), dynMesh->vertices.GetData());

    dynMesh->aabb.min = NX_VEC3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    dynMesh->aabb.max = NX_VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const NX_Vertex3D& vertex : dynMesh->vertices) {
        dynMesh->aabb.min = NX_Vec3Min(dynMesh->aabb.min, vertex.position);
        dynMesh->aabb.max = NX_Vec3Max(dynMesh->aabb.max, vertex.position);
    }
}

void NX_SetDynamicMeshTexCoord(NX_DynamicMesh* dynMesh, NX_Vec2 texcoord)
{
    dynMesh->current.texcoord = texcoord;
}

void NX_SetDynamicMeshNormal(NX_DynamicMesh* dynMesh, NX_Vec3 normal)
{
    dynMesh->current.normal = normal;
}

void NX_SetDynamicMeshTangent(NX_DynamicMesh* dynMesh, NX_Vec4 tangent)
{
    dynMesh->current.tangent = tangent;
}

void NX_SetDynamicMeshColor(NX_DynamicMesh* dynMesh, NX_Color color)
{
    dynMesh->current.color = color;
}

void NX_AddDynamicMeshVertex(NX_DynamicMesh* dynMesh, NX_Vec3 position)
{
    dynMesh->current.position = position;
    dynMesh->vertices.PushBack(dynMesh->current);
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
