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
 * Defines the bone structure, reference poses, and inverse bind matrices
 * required for skeletal animation. The skeleton provides both local and
 * global bind poses used during skinning and animation playback.
 */
typedef struct NX_Skeleton {

    NX_BoneInfo* bones;       ///< Array of bone descriptors defining the hierarchy and names.
    int boneCount;            ///< Total number of bones in the skeleton.

    NX_Mat4* boneOffsets;     ///< Inverse bind matrices, one per bone. Transform vertices from mesh space to bone space (used in skinning).
    NX_Mat4* bindLocal;       ///< Bind pose transforms in local bone space (relative to parent).
    NX_Mat4* bindPose;        ///< Bind pose transforms in model space (global). Used as the default pose when not animated.

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
