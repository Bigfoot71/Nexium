/* NX_MeshData.h -- API declaration for Nexium's mesh data module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MESH_DATA_H
#define NX_MESH_DATA_H

#include "./NX_Vertex.h"
#include "./NX_Shape.h"
#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Represents a mesh stored in CPU memory.
 *
 * NX_MeshData is the CPU-side container of a mesh. It stores vertex and index data,
 * and provides utility functions to generate, transform, and process geometry before
 * uploading it to the GPU as an NX_Mesh.
 *
 * Think of it as a toolbox for procedural or dynamic mesh generation on the CPU.
 */
typedef struct NX_MeshData {
    NX_Vertex3D* vertices;      ///< Pointer to vertex data in CPU memory.
    uint32_t* indices;          ///< Pointer to index data in CPU memory.
    int vertexCount;            ///< Number of vertices.
    int indexCount;             ///< Number of indices.
} NX_MeshData;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates an empty mesh data container.
 * @param vertexCount Number of vertices to allocate.
 * @param indexCount Number of indices to allocate.
 * @return A new NX_MeshData instance with allocated memory.
 */
NXAPI NX_MeshData NX_CreateMeshData(int vertexCount, int indexCount);

/**
 * @brief Releases memory used by a mesh data container.
 * @param meshData Pointer to the NX_MeshData to destroy.
 */
NXAPI void NX_DestroyMeshData(NX_MeshData* meshData);

/**
 * @brief Creates a deep copy of an existing mesh data container.
 * @param meshData Source mesh data to duplicate.
 * @return A new NX_MeshData containing a copy of the source data.
 */
NXAPI NX_MeshData NX_DuplicateMeshData(const NX_MeshData* meshData);

/**
 * @brief Merges two mesh data containers into a single one.
 * @param a First mesh data.
 * @param b Second mesh data.
 * @return A new NX_MeshData containing the merged geometry.
 */
NXAPI NX_MeshData NX_MergeMeshData(const NX_MeshData* a, const NX_MeshData* b);

/**
 * @brief Translates all vertices by a given offset.
 * @param meshData Mesh data to modify.
 * @param translation Offset to apply to all vertex positions.
 */
NXAPI void NX_TranslateMeshData(NX_MeshData* meshData, NX_Vec3 translation);

/**
 * @brief Rotates all vertices using a quaternion.
 * @param meshData Mesh data to modify.
 * @param rotation Quaternion representing the rotation.
 */
NXAPI void NX_RotateMeshData(NX_MeshData* meshData, NX_Quat rotation);

/**
 * @brief Scales all vertices by given factors.
 * @param meshData Mesh data to modify.
 * @param scale Scaling factors for each axis.
 */
NXAPI void NX_ScaleMeshData(NX_MeshData* meshData, NX_Vec3 scale);

/**
 * @brief Generates planar UV coordinates.
 * @param meshData Mesh data to modify.
 * @param uvScale Scaling factors for UV coordinates.
 * @param axis Axis along which to project the planar mapping.
 */
NXAPI void NX_GenMeshDataUVsPlanar(NX_MeshData* meshData, NX_Vec2 uvScale, NX_Vec3 axis);

/**
 * @brief Generates spherical UV coordinates.
 * @param meshData Mesh data to modify.
 */
NXAPI void NX_GenMeshDataUVsSpherical(NX_MeshData* meshData);

/**
 * @brief Generates cylindrical UV coordinates.
 * @param meshData Mesh data to modify.
 */
NXAPI void NX_GenMeshDataUVsCylindrical(NX_MeshData* meshData);

/**
 * @brief Computes vertex normals from triangle geometry.
 * @param meshData Mesh data to modify.
 */
NXAPI void NX_GenMeshDataNormals(NX_MeshData* meshData);

/**
 * @brief Computes vertex tangents based on existing normals and UVs.
 * @param meshData Mesh data to modify.
 */
NXAPI void NX_GenMeshDataTangents(NX_MeshData* meshData);

/**
 * @brief Calculates the axis-aligned bounding box of the mesh.
 * @param meshData Mesh data to analyze.
 * @return The computed bounding box.
 */
NXAPI NX_BoundingBox3D NX_CalculateMeshDataAABB(const NX_MeshData* meshData);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_MESH_DATA_H
