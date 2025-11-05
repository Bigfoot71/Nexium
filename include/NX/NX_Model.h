/* NX_Model.h -- API declaration for Nexium's model module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MODEL_H
#define NX_MODEL_H

#include "./NX_Animation.h"
#include "./NX_Material.h"
#include "./NX_Skeleton.h"
#include "./NX_Mesh.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Animation Update modes.
 *
 * Controls wether to allow external animation matrices
 */
typedef enum NX_AnimMode {
    NX_ANIM_INTERNAL,           ///< Default animation solution
    NX_ANIM_CUSTOM,             ///< User supplied matrices
} NX_AnimMode;

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

    const NX_Animation* anim;           ///< Pointer to the currently assigned animation for this model (optional).
    NX_Mat4* boneOverride;              ///< Array of matrices we'll use if we have it instead of internal calculations, Used in skinning.
    NX_AnimMode animMode;               ///< Animation mode for the model; specifies whether to use the model’s animation and frame or the boneOverride.
    float animFrame;                    ///< Current animation frame index. Used for sampling bone poses from the animation.

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
 * @param model Pointer to the NX_Model to update.
 * @param updateMeshAABBs If true, also updates each mesh's bounding box before updating the model AABB.
 */
NXAPI void NX_UpdateModelAABB(NX_Model* model, bool updateMeshAABBs);

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
