/* NX_Random.h -- API declaration for Nexium's random module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RANDOM_H
#define NX_RANDOM_H

#include "./NX_API.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// ============================================================================
// TYPES DEFINITIONS
// ============================================================================

/**
 * @brief PCG32 pseudo-random number generator state
 *
 * This structure contains the internal state for a PCG32 random number generator.
 * It can be allocated on the stack for temporary use or managed through the pool
 * allocation system for shared/persistent generators.
 *
 * @note The generator state should be initialized using NX_CreateRandGenTemp() for stack allocation
 *       or NX_CreateRandGen() for pool allocation. Direct manipulation of the fields is not
 *       recommended as it may compromise the generator's statistical properties.
 *
 * @see NX_CreateRandGenTemp() for stack-based temporary generators
 * @see NX_CreateRandGen() for pool-managed persistent generators
 */
typedef struct NX_RandGen {
    uint64_t state;     ///< Internal 64-bit state
    uint64_t inc;       ///< Increment (must be odd)
} NX_RandGen;

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/** @defgroup Random Random Functions
 *  Pseudo-random number generation using PCG32 algorithm.
 *  All functions accept a generator parameter that can be NULL to use the default generator.
 *  @{
 */

/**
 * @brief Create a random generator initialized with the given seed
 * @param seed 64-bit seed value
 * @return Pointer to the created random generator, or NULL on failure
 */
NXAPI NX_RandGen* NX_CreateRandGen(uint64_t seed);

/**
 * @brief Get a temporary random generator with the given seed
 * @param seed 64-bit seed value
 * @return Temporary generator (stack allocated)
 * @note This generator is only valid within the current scope
 */
NXAPI NX_RandGen NX_CreateRandGenTemp(uint64_t seed);

/**
 * @brief Destroy the specified random generator
 * @param generator Pointer to the random generator instance to destroy
 * @note Can be called with NULL (no-op)
 */
NXAPI void NX_DestroyRandGen(NX_RandGen* generator);

/**
 * @brief Seed the specified random number generator
 * @param generator Pointer to the generator to seed, or NULL to seed the default generator
 * @param seed 64-bit seed value
 */
NXAPI void NX_SetRandGenSeed(NX_RandGen* generator, uint64_t seed);

/**
 * @brief Generate a random boolean value
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random boolean value
 */
NXAPI bool NX_RandBool(NX_RandGen* generator);

/**
 * @brief Generate a random signed 32-bit integer
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random int32_t value
 */
NXAPI int32_t NX_RandInt(NX_RandGen* generator);

/**
 * @brief Generate a random unsigned 32-bit integer
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random uint32_t value
 */
NXAPI uint32_t NX_RandUint(NX_RandGen* generator);

/**
 * @brief Generate a random float in the range [0.0, 1.0)
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random float value
 */
NXAPI float NX_RandFloat(NX_RandGen* generator);

/**
 * @brief Generate a random integer in a given range [min, max]
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random integer in range, or min if min >= max
 */
NXAPI int NX_RandRangeInt(NX_RandGen* generator, int min, int max);

/**
 * @brief Generate a random unsigned integer in a given range [min, max]
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random unsigned integer in range, or min if min >= max
 */
NXAPI uint32_t NX_RandRangeUint(NX_RandGen* generator, uint32_t min, uint32_t max);

/**
 * @brief Generate a random float in a given range [min, max)
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @param min Minimum value (inclusive)
 * @param max Maximum value (exclusive)
 * @return Random float in range
 */
NXAPI float NX_RandRangeFloat(NX_RandGen* generator, float min, float max);

/**
 * @brief Shuffle an array of elements using Fisher-Yates algorithm
 *
 * Behavior depends on the element size:
 * - If element size <= 64 bytes, a temporary stack array is used.
 * - If element size > 64 bytes, a heap allocation is used for the shuffle.
 *
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @param array Pointer to array to shuffle
 * @param element_size Size of each element in bytes
 * @param count Number of elements in the array
 * @note Does nothing if array is NULL, count <= 1, or element_size is 0
 */
NXAPI void NX_RandShuffle(NX_RandGen* generator, void* array, size_t element_size, size_t count);

/** @} */ // Random

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_RANDOM_H
