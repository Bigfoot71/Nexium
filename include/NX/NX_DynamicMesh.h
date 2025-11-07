/* NX_DynamicMesh.h -- API declaration for Nexium's dynamic mesh module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_DYNAMIC_MESH_H
#define NX_DYNAMIC_MESH_H

#include "./NX_Camera.h"
#include "./NX_Vertex.h"
#include "./NX_Light.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Flags controlling automatic attribute generation for dynamic meshes.
 *
 * These flags define which additional vertex attributes are automatically
 * generated when finalizing a dynamic mesh.
 */
typedef uint32_t NX_DynamicMeshFlags;

#define NX_DYNAMIC_MESH_GEN_NORMALS     (1 << 0)    ///< Automatically generate vertex normals.
#define NX_DYNAMIC_MESH_GEN_TANGENTS    (1 << 1)    ///< Automatically generate vertex tangents.

/**
 * @brief Opaque handle to a GPU dynamic mesh.
 *
 * Represents a mesh whose vertex data can be modified or rebuilt each frame.
 */
typedef struct NX_DynamicMesh NX_DynamicMesh;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates a dynamic mesh.
 *
 * Create a dynamic mesh that can be rebuilt each frame.
 *
 * @param initialCapacity Initial number of vertices to pre-allocate (must be > 0).
 *                        This value is only informative; memory will be reallocated if needed.
 * @return Pointer to a new NX_DynamicMesh, or NULL if creation fails.
 */
NXAPI NX_DynamicMesh* NX_CreateDynamicMesh(size_t initialCapacity);

/**
 * @brief Destroys a dynamic mesh.
 *
 * Releases all GPU and CPU resources associated with the mesh.
 *
 * @param dynMesh Pointer to the dynamic mesh to destroy.
 */
NXAPI void NX_DestroyDynamicMesh(NX_DynamicMesh* dynMesh);

/**
 * @brief Begins recording geometry for a dynamic mesh.
 *
 * Initializes a new immediate-mode geometry build for the given dynamic mesh.
 * Any previously recorded geometry is discarded. The primitive type and
 * optional generation flags can change between frames.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param type Primitive type to use for the new geometry.
 * @param flags Combination of NX_DynamicMeshFlags controlling automatic attribute generation.
 */
NXAPI void NX_BeginDynamicMesh(NX_DynamicMesh* dynMesh, NX_PrimitiveType type, NX_DynamicMeshFlags flags);

/**
 * @brief Ends recording and uploads geometry to the GPU.
 *
 * Reallocates memory on the GPU if necessary.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 */
NXAPI void NX_EndDynamicMesh(NX_DynamicMesh* dynMesh);

/**
 * @brief Sets the texture coordinate for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param texcoord 2D texture coordinate to assign.
 */
NXAPI void NX_SetDynamicMeshTexCoord(NX_DynamicMesh* dynMesh, NX_Vec2 texcoord);

/**
 * @brief Sets the normal vector for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param normal 3D normal vector to assign.
 */
NXAPI void NX_SetDynamicMeshNormal(NX_DynamicMesh* dynMesh, NX_Vec3 normal);

/**
 * @brief Sets the tangent vector for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param tangent 4D tangent vector to assign.
 */
NXAPI void NX_SetDynamicMeshTangent(NX_DynamicMesh* dynMesh, NX_Vec4 tangent);

/**
 * @brief Sets the color for the next vertex.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param color RGBA color to assign.
 */
NXAPI void NX_SetDynamicMeshColor(NX_DynamicMesh* dynMesh, NX_Color color);

/**
 * @brief Adds a vertex to the dynamic mesh.
 *
 * The current attributes (position, normal, tangent, texcoord, color) are used.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param position 3D position of the vertex.
 */
NXAPI void NX_AddDynamicMeshVertex(NX_DynamicMesh* dynMesh, NX_Vec3 position);

/**
 * @brief Sets the shadow casting mode for a dynamic mesh.
 *
 * Default is NX_SHADOW_CAST_ENABLED.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param mode Shadow casting mode to apply.
 */
NXAPI void NX_SetDynamicMeshShadowCastMode(NX_DynamicMesh* dynMesh, NX_ShadowCastMode mode);

/**
 * @brief Sets the shadow face mode for a dynamic mesh.
 *
 * Default is NX_SHADOW_FACE_AUTO.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param mode Shadow face mode to apply.
 */
NXAPI void NX_SetDynamicMeshShadowFaceMode(NX_DynamicMesh* dynMesh, NX_ShadowFaceMode mode);

/**
 * @brief Sets the layer mask for a dynamic mesh.
 *
 * Default is NX_LAYER_01.
 *
 * @param dynMesh Pointer to the dynamic mesh (cannot be NULL).
 * @param mask Layer mask to assign.
 */
NXAPI void NX_SetDynamicMeshLayerMask(NX_DynamicMesh* dynMesh, NX_Layer mask);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_DYNAMIC_MESH_H
