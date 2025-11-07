/* NX_Mesh.h -- API declaration for Nexium's mesh module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MESH_H
#define NX_MESH_H

#include "./NX_MeshData.h"
#include "./NX_Camera.h"
#include "./NX_Vertex.h"
#include "./NX_Shape.h"
#include "./NX_Light.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Represents a 3D mesh.
 *
 * Stores vertex and index data, shadow casting settings, bounding box, and layer information.
 * Can represent a static or skinned mesh.
 */
typedef struct NX_Mesh {
    NX_VertexBuffer3D* buffer;          ///< GPU vertex buffer for rendering.
    NX_ShadowCastMode shadowCastMode;   ///< Shadow casting mode for the mesh.
    NX_ShadowFaceMode shadowFaceMode;   ///< Which faces are rendered into the shadow map.
    NX_PrimitiveType primitiveType;     ///< Type of primitive that constitutes the vertices.
    NX_BoundingBox3D aabb;              ///< Axis-Aligned Bounding Box in local space.
    NX_Layer layerMask;                 ///< Bitfield indicating the rendering layer(s) of this mesh.
} NX_Mesh;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates a 3D mesh from CPU-side mesh data.
 * @param type Primitive type used to interpret vertex data.
 * @param meshData Pointer to the NX_MeshData containing vertices and indices (cannot be NULL).
 * @param aabb Optional pointer to a bounding box. If NULL, it will be computed automatically.
 * @return Pointer to a newly created NX_Mesh.
 * @note The function copies all vertex and index data into GPU buffers.
 */
NXAPI NX_Mesh* NX_CreateMesh(NX_PrimitiveType type, const NX_MeshData* meshData, const NX_BoundingBox3D* aabb);

/**
 * @brief Destroys a 3D mesh and frees its resources.
 * @param mesh Pointer to the NX_Mesh to destroy.
 */
NXAPI void NX_DestroyMesh(NX_Mesh* mesh);

/**
 * @brief Generates a quad mesh.
 * @param size Width and height of the quad.
 * @param subDiv Subdivision along X and Y axes.
 * @param normal Normal vector for the quad surface.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshQuad(NX_Vec2 size, NX_IVec2 subDiv, NX_Vec3 normal);

/**
 * @brief Generates a cube mesh.
 * @param size Dimensions along X, Y, Z.
 * @param subDiv Subdivision along each axis.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshCube(NX_Vec3 size, NX_IVec3 subDiv);

/**
 * @brief Generates a sphere mesh.
 * @param radius Sphere radius.
 * @param slices Number of slices (longitudinal divisions).
 * @param rings Number of rings (latitudinal divisions).
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshSphere(float radius, int slices, int rings);

/**
 * @brief Generates a cylinder mesh.
 * @param topRadius Radius of the top cap.
 * @param bottomRadius Radius of the bottom cap.
 * @param height Height of the cylinder.
 * @param slices Number of slices around the cylinder.
 * @param rings Number of subdivisions along the height.
 * @param topCap Whether to generate the top cap.
 * @param bottomCap Whether to generate the bottom cap.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshCylinder(float topRadius, float bottomRadius, float height, int slices, int rings, bool topCap, bool bottomCap);

/**
 * @brief Generates a capsule mesh.
 * @param radius Capsule radius.
 * @param height Capsule height.
 * @param slices Number of slices around the capsule.
 * @param rings Number of rings along the capsule.
 * @return Pointer to a newly generated NX_Mesh.
 */
NXAPI NX_Mesh* NX_GenMeshCapsule(float radius, float height, int slices, int rings);

/**
 * @brief Uploads the mesh data currently stored in RAM to the GPU.
 * @param mesh Pointer to the NX_Mesh to update.
 * @note Useful after modifying vertices or indices to update the GPU buffers.
 */
NXAPI void NX_UpdateMeshBuffer(NX_Mesh* mesh);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_MESH_H
