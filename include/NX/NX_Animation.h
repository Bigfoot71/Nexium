/* NX_Animation.h -- API declaration for Nexium's animation module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_ANIMATION_H
#define NX_ANIMATION_H

#include "./NX_Skeleton.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Represents a skeletal animation for a model.
 *
 * This structure holds the animation data for a skinned model,
 * including per-frame bone transformation poses.
 */
typedef struct NX_Animation {

    NX_BoneInfo* bones;             ///< Array of bone metadata (name, parent index, etc.) defining the skeleton hierarchy.
    int boneCount;                  ///< Number of bones in the skeleton affected by this animation.

    NX_Transform** frameLocalPoses; ///< 2D array [frame][bone]. Local bone transforms (TRS relative to parent).
    NX_Mat4** frameGlobalPoses;     ///< 2D array [frame][bone]. Global bone matrices (relative to model space).
    int frameCount;                 ///< Total number of frames in the animation sequence.

    char name[32];                  ///< Name identifier for the animation (e.g., "Walk", "Jump").

} NX_Animation;

/**
 * @brief Represents a collection of skeletal animations sharing the same skeleton.
 *
 * Holds multiple animations that can be applied to compatible models or skeletons.
 * Typically loaded together from a single 3D model file (e.g., GLTF, FBX) containing several animation clips.
 */
typedef struct NX_AnimationLib {
    NX_Animation* animations;       ///< Array of animations included in this library.
    int count;                      ///< Number of animations contained in the library.
} NX_AnimationLib;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Loads animations from a model file.
 * @param filePath Path to the model file containing animations.
 * @param targetFrameRate Desired frame rate (FPS) for sampling the animations.
 * @return Pointer to an array of NX_Animation, or NULL on failure.
 * @note Free the returned array using NX_DestroyAnimationLib().
 */
NXAPI NX_AnimationLib* NX_LoadAnimationLib(const char* filePath, int targetFrameRate);

/**
 * @brief Loads animations from memory data.
 * @param data Pointer to memory buffer containing model animation data.
 * @param size Size of the buffer in bytes.
 * @param hint Hint on the model format (can be NULL).
 * @param targetFrameRate Desired frame rate (FPS) for sampling the animations.
 * @return Pointer to an array of NX_Animation, or NULL on failure.
 * @note Free the returned array using NX_DestroyAnimationLib().
 */
NXAPI NX_AnimationLib* NX_LoadAnimationLibFromData(const void* data, unsigned int size, const char* hint, int targetFrameRate);

/**
 * @brief Frees memory allocated for model animations.
 * @param animLib Pointer to the animation library to free.
 */
NXAPI void NX_DestroyAnimationLib(NX_AnimationLib* animLib);

/**
 * @brief Finds a named animation in an array of animations.
 * @param animLib Pointer to the animation library.
 * @param name Name of the animation to find (case-sensitive).
 * @return Pointer to the matching animation, or NULL if not found.
 */
NXAPI NX_Animation* NX_GetAnimation(const NX_AnimationLib* animLib, const char* name);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_ANIMATION_H
