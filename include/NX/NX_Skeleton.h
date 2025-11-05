/* NX_Skeleton.h -- API declaration for Nexium's skeleton module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_SKELETON_H
#define NX_SKELETON_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

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
 * @brief Represents a skeletal hierarchy used for skinning.
 *
 * Defines the bone structure and reference poses of a model.
 * The skeleton provides the hierarchical bone layout, the bind pose,
 * and the inverse bind matrices required for vertex skinning and animation playback.
 */
typedef struct NX_Skeleton {

    NX_BoneInfo* bones;                 ///< Bones information (skeleton). Defines the hierarchy and names of bones.
    int boneCount;                      ///< Number of bones.

    NX_Mat4* boneBindPose;              ///< Array of matrices representing the bind pose of the model, this is the pose used by default for non-animated skinned models.
    NX_Mat4* boneOffsets;               ///< Array of offset (inverse bind) matrices, one per bone. Transforms mesh-space vertices to bone space. Used in skinning.

} NX_Skeleton;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Loads a skeleton hierarchy from a 3D model file.
 *
 * Skeletons are automatically loaded when importing a model,
 * but can be loaded manually for advanced use cases.
 *
 * @param filePath Path to the model file containing the skeleton data.
 * @return Pointer to a newly loaded NX_Skeleton, or NULL on failure.
 */
NXAPI NX_Skeleton* NX_LoadSkeleton(const char* filePath);

/**
 * @brief Loads a skeleton hierarchy from memory data.
 *
 * Allows manual loading of skeletons directly from a memory buffer.
 * Typically used for advanced or custom asset loading workflows.
 *
 * @param data Pointer to the memory buffer containing skeleton data.
 * @param size Size of the memory buffer in bytes.
 * @param hint Optional format hint (can be NULL).
 * @return Pointer to a newly loaded NX_Skeleton, or NULL on failure.
 */
NXAPI NX_Skeleton* NX_LoadSkeletonFromData(const void* data, unsigned int size, const char* hint);

/**
 * @brief Frees the memory allocated for a skeleton.
 *
 * @param skeleton Pointer to the NX_Skeleton to destroy.
 */
NXAPI void NX_DestroySkeleton(NX_Skeleton* skeleton);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_SKELETON_H
