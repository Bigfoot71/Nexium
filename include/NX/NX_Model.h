/* NX_Model.h -- API declaration for Nexium's model module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MODEL_H
#define NX_MODEL_H

#include "./NX_AnimationPlayer.h"
#include "./NX_Material.h"
#include "./NX_Skeleton.h"
#include "./NX_Mesh.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Represents a complete 3D model with meshes and materials.
 *
 * Contains multiple meshes and their associated materials, along with animation or bounding information.
 */
typedef struct NX_Model {

    NX_Mesh** meshes;                   ///< Array of meshes composing the model.
    NX_Material* materials;             ///< Array of materials used by the model.
    int* meshMaterials;                 ///< Array of material indices, one per mesh.

    int meshCount;                      ///< Number of meshes.
    int materialCount;                  ///< Number of materials.

    NX_BoundingBox3D aabb;              ///< Axis-Aligned Bounding Box encompassing the whole model.
    NX_Skeleton* skeleton;              ///< Skeleton hierarchy and bind pose used for skinning (NULL if non-skinned).

    NX_AnimationPlayer* player;         ///< Animation player controlling the skeleton. If NULL the model uses its bind pose.

} NX_Model;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Loads a 3D model from a file.
 * @param filePath Path to the model file.
 * @return Pointer to a newly loaded NX_Model containing meshes and materials.
 */
NXAPI NX_Model* NX_LoadModel(const char* filePath);

/**
 * @brief Loads a 3D model from memory.
 * @param data Pointer to memory buffer containing model data.
 * @param size Size of the memory buffer in bytes.
 * @param hint Hint on the model format (can be NULL).
 * @return Pointer to a newly loaded NX_Model containing meshes and materials.
 */
NXAPI NX_Model* NX_LoadModelFromData(const void* data, size_t size, const char* hint);

/**
 * @brief Destroys a 3D model and frees its resources.
 * @param model Pointer to the NX_Model to destroy.
 */
NXAPI void NX_DestroyModel(NX_Model* model);

/**
 * @brief Updates the axis-aligned bounding box (AABB) of a model.
 *
 * Recalculates the model's bounding box by combining the AABBs
 * of all meshes it contains. This function does not modify
 * individual mesh bounding boxes.
 *
 * @param model Pointer to the NX_Model whose bounding box will be updated.
 */
NXAPI void NX_UpdateModelAABB(NX_Model* model);

/**
 * @brief Scales the axis-aligned bounding box (AABB) of a model by a given factor.
 * @param model Pointer to the NX_Model whose AABB will be scaled.
 * @param scale Scaling factor to apply to the AABB.
 * @param scaleMeshAABBs If true, also scales the AABBs of each mesh before scaling the model AABB.
 */
NXAPI void NX_ScaleModelAABB(NX_Model* model, float scale, bool scaleMeshAABBs);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_MODEL_H
