/* NX_AnimationPlayer.h -- API declaration for Nexium's animation player module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_ANIMATION_PLAYER_H
#define NX_ANIMATION_PLAYER_H

#include "./NX_Animation.h"
#include "./NX_Skeleton.h"

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief Describes the playback state of a single animation.
 *
 * Each state tracks the current playback time, blending weight,
 * and looping behavior for one animation within a player.
 */
typedef struct NX_AnimationState {
    float currentTime;  ///< Current playback time in animation ticks.
    float weight;       ///< Blending weight of this animation (0.0-1.0).
    bool loop;          ///< True to enable looping playback.
} NX_AnimationState;

/**
 * @brief Controls playback and blending of animations for a skeleton.
 *
 * The animation player manages multiple animation states from a given
 * animation library and computes the blended pose for the associated skeleton.
 * On each update, it advances internal timers, interpolates keyframes,
 * blends active animations according to their weights, and updates the
 * current skeleton pose.
 */
typedef struct NX_AnimationPlayer {
    const NX_AnimationLib* animLib; ///< Animation library providing available animations.
    const NX_Skeleton* skeleton;    ///< Target skeleton to animate.
    NX_AnimationState* states;      ///< Array of active animation states.
    NX_Mat4* currentPose;           ///< Array of bone transforms representing the blended pose.
} NX_AnimationPlayer;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Creates a new animation player for a skeleton and animation library.
 *
 * Allocates internal structures for managing animation states and poses.
 *
 * @param skeleton Pointer to the target skeleton.
 * @param animLib Pointer to the animation library containing available animations.
 * @return Pointer to a newly created animation player, or NULL on failure.
 */
NXAPI NX_AnimationPlayer* NX_CreateAnimationPlayer(const NX_Skeleton* skeleton, const NX_AnimationLib* animLib);

/**
 * @brief Destroys an animation player and frees its allocated resources.
 *
 * @param player Pointer to the animation player to destroy.
 */
NXAPI void NX_DestroyAnimationPlayer(NX_AnimationPlayer* player);

/**
 * @brief Updates the animation player by advancing time and blending animations.
 *
 * This function interpolates keyframes, blends all active animation states,
 * and updates the current skeleton pose. The time step `dt` can be scaled
 * to modify playback speed.
 *
 * @param player Pointer to the animation player.
 * @param dt Delta time since the last update, in seconds.
 */
NXAPI void NX_UpdateAnimationPlayer(NX_AnimationPlayer* player, float dt);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_ANIMATION_PLAYER_H
