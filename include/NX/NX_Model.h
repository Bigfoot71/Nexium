/* NX_Model.h -- API declaration for Nexium's model module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MODEL_H
#define NX_MODEL_H

#include "./NX_Material.h"
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
 * @brief Stores bone information for skeletal animation.
 *
 * Contains the bone name and the index of its parent bone.
 */
typedef struct NX_BoneInfo {
    char name[32];   ///< Bone name (max 31 characters + null terminator).
    int parent;      ///< Index of the parent bone (-1 if root).
} NX_BoneInfo;

/**
 * @brief Represents a skeletal animation for a model.
 *
 * This structure holds the animation data for a skinned model,
 * including per-frame bone transformation poses.
 */
typedef struct NX_Animation {

    int boneCount;                  ///< Number of bones in the skeleton affected by this animation.
    int frameCount;                 ///< Total number of frames in the animation sequence.

    NX_BoneInfo* bones;             ///< Array of bone metadata (name, parent index, etc.) defining the skeleton hierarchy.

    NX_Mat4** frameGlobalPoses;     ///< 2D array [frame][bone]. Global bone matrices (relative to model space).
    NX_Transform** frameLocalPoses; ///< 2D array [frame][bone]. Local bone transforms (TRS relative to parent).

    char name[32];                  ///< Name identifier for the animation (e.g., "Walk", "Jump").

} NX_Animation;

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

    NX_BoundingBox3D aabb;                ///< Axis-Aligned Bounding Box encompassing the whole model.

    NX_Mat4* boneOverride;              ///< Array of matrices we'll use if we have it instead of internal calculations, Used in skinning.
    NX_Mat4* boneBindPose;              ///< Array of matrices representing the bind pose of the model, this is the pose used by default for non-animated skinned models.
    NX_Mat4* boneOffsets;               ///< Array of offset (inverse bind) matrices, one per bone. Transforms mesh-space vertices to bone space. Used in skinning.

    NX_BoneInfo* bones;                 ///< Bones information (skeleton). Defines the hierarchy and names of bones.
    int boneCount;                      ///< Number of bones.

    const NX_Animation* anim;           ///< Pointer to the currently assigned animation for this model (optional).
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

/**
 * @brief Loads animations from a model file.
 * @param filePath Path to the model file containing animations.
 * @param animCount Pointer to an int that receives the number of animations loaded.
 * @param targetFrameRate Desired frame rate (FPS) for sampling the animations.
 * @return Pointer to an array of NX_Animation, or NULL on failure.
 * @note Free the returned array using NX_DestroyAnimations().
 */
NXAPI NX_Animation** NX_LoadAnimations(const char* filePath, int* animCount, int targetFrameRate);

/**
 * @brief Loads animations from memory data.
 * @param data Pointer to memory buffer containing model animation data.
 * @param size Size of the buffer in bytes.
 * @param hint Hint on the model format (can be NULL).
 * @param animCount Pointer to an int that receives the number of animations loaded.
 * @param targetFrameRate Desired frame rate (FPS) for sampling the animations.
 * @return Pointer to an array of NX_Animation, or NULL on failure.
 * @note Free the returned array using NX_DestroyAnimations().
 */
NXAPI NX_Animation** NX_LoadAnimationFromData(const void* data, unsigned int size, const char* hint, int* animCount, int targetFrameRate);

/**
 * @brief Frees memory allocated for model animations.
 * @param animations Pointer to the animation array to free.
 * @param animCount Number of animations in the array.
 */
NXAPI void NX_DestroyAnimations(NX_Animation** animations, int animCount);

/**
 * @brief Finds a named animation in an array of animations.
 * @param animations Array of NX_Animation pointers.
 * @param animCount Number of animations in the array.
 * @param name Name of the animation to find (case-sensitive).
 * @return Pointer to the matching animation, or NULL if not found.
 */
NXAPI NX_Animation* NX_GetAnimation(NX_Animation** animations, int animCount, const char* name);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_MODEL_H
