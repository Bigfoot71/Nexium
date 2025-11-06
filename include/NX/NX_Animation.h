/* NX_Animation.h -- API declaration for Nexium's animation module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_ANIMATION_H
#define NX_ANIMATION_H

#include "./NX_Math.h"
#include "./NX_API.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Represents a single 3D vector keyframe used in animation.
 *
 * Stores a position or scale value and the time at which it occurs
 * in the animation timeline.
 */
typedef struct NX_Vec3Key {
    NX_Vec3 value;  ///< Keyframe value (position or scale) in local space.
    float time;     ///< Time of the keyframe, in animation ticks.
} NX_Vec3Key;


/**
 * @brief Represents a quaternion rotation keyframe used in animation.
 *
 * Stores a rotation value and the time at which it occurs.
 */
typedef struct NX_QuatKey {
    NX_Quat value;  ///< Keyframe value representing a bone rotation.
    float time;     ///< Time of the keyframe, in animation ticks.
} NX_QuatKey;


/**
 * @brief Animation channel describing how a single bone transforms over time.
 *
 * Each channel contains position, rotation, and scale keyframes for one bone.
 * During playback, these keys are interpolated to compute the bone's local transform.
 */
typedef struct NX_AnimationChannel {
    NX_Vec3Key* positionKeys;       ///< Array of translation keyframes.
    NX_QuatKey* rotationKeys;       ///< Array of rotation keyframes.
    NX_Vec3Key* scaleKeys;          ///< Array of scaling keyframes.
    uint32_t positionKeyCount;      ///< Number of translation keyframes.
    uint32_t rotationKeyCount;      ///< Number of rotation keyframes.
    uint32_t scaleKeyCount;         ///< Number of scaling keyframes.
    int boneIndex;                  ///< Index of the bone affected by this channel.
} NX_AnimationChannel;


/**
 * @brief Represents a skeletal animation for a model.
 *
 * Contains all animation channels required to animate a skeleton.
 * Each channel corresponds to one bone and defines its transformation
 * (translation, rotation, scale) over time.
 */
typedef struct NX_Animation {
    NX_AnimationChannel* channels;  ///< Array of animation channels, one per animated bone.
    uint32_t channelCount;          ///< Total number of channels in this animation.
    float ticksPerSecond;           ///< Playback rate; number of animation ticks per second.
    float duration;                 ///< Total length of the animation, in ticks.
    int boneCount;                  ///< Number of bones in the target skeleton.
    char name[32];                  ///< Animation name (null-terminated string).
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
NXAPI NX_AnimationLib* NX_LoadAnimationLib(const char* filePath);

/**
 * @brief Loads animations from memory data.
 * @param data Pointer to memory buffer containing model animation data.
 * @param size Size of the buffer in bytes.
 * @param hint Hint on the model format (can be NULL).
 * @param targetFrameRate Desired frame rate (FPS) for sampling the animations.
 * @return Pointer to an array of NX_Animation, or NULL on failure.
 * @note Free the returned array using NX_DestroyAnimationLib().
 */
NXAPI NX_AnimationLib* NX_LoadAnimationLibFromData(const void* data, unsigned int size, const char* hint);

/**
 * @brief Frees memory allocated for model animations.
 * @param animLib Pointer to the animation library to free.
 */
NXAPI void NX_DestroyAnimationLib(NX_AnimationLib* animLib);

/**
 * @brief Retrieves the index of a named animation within an animation library.
 * @param animLib Pointer to the animation library.
 * @param name Name of the animation to look for (case-sensitive).
 * @return Zero-based index of the matching animation, or -1 if not found.
 */
NXAPI int NX_GetAnimationIndex(const NX_AnimationLib* animLib, const char* name);

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
