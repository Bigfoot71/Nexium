/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef HP_RAND_H
#define HP_RAND_H

#include "./HP_Platform.h"

#include <stdint.h>
#include <stddef.h>

/* === Structures === */

/**
 * @brief PCG32 pseudo-random number generator state
 *
 * This structure contains the internal state for a PCG32 random number generator.
 * It can be allocated on the stack for temporary use or managed through the pool
 * allocation system for shared/persistent generators.
 *
 * @note The generator state should be initialized using HP_CreateRandGenTemp() for stack allocation
 *       or HP_CreateRandGen() for pool allocation. Direct manipulation of the fields is not
 *       recommended as it may compromise the generator's statistical properties.
 *
 * @see HP_CreateRandGenTemp() for stack-based temporary generators
 * @see HP_CreateRandGen() for pool-managed persistent generators
 */
typedef struct HP_RandGen {
    uint64_t state;     ///< Internal 64-bit state
    uint64_t inc;       ///< Increment (must be odd)
} HP_RandGen;

/* === API Functions === */

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
HPAPI HP_RandGen* HP_CreateRandGen(uint64_t seed);

/**
 * @brief Get a temporary random generator with the given seed
 * @param seed 64-bit seed value
 * @return Temporary generator (stack allocated)
 * @note This generator is only valid within the current scope
 */
HPAPI HP_RandGen HP_CreateRandGenTemp(uint64_t seed);

/**
 * @brief Destroy the specified random generator
 * @param generator Pointer to the random generator instance to destroy
 * @note Can be called with NULL (no-op)
 */
HPAPI void HP_DestroyRandGen(HP_RandGen* generator);

/**
 * @brief Seed the specified random number generator
 * @param generator Pointer to the generator to seed, or NULL to seed the default generator
 * @param seed 64-bit seed value
 */
HPAPI void HP_SetRandGenSeed(HP_RandGen* generator, uint64_t seed);

/**
 * @brief Generate a random boolean value
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random boolean value
 */
HPAPI bool HP_RandBool(HP_RandGen* generator);

/**
 * @brief Generate a random signed 32-bit integer
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random int32_t value
 */
HPAPI int32_t HP_RandInt(HP_RandGen* generator);

/**
 * @brief Generate a random unsigned 32-bit integer
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random uint32_t value
 */
HPAPI uint32_t HP_RandUint(HP_RandGen* generator);

/**
 * @brief Generate a random float in the range [0.0, 1.0)
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @return Random float value
 */
HPAPI float HP_RandFloat(HP_RandGen* generator);

/**
 * @brief Generate a random integer in a given range [min, max]
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random integer in range, or min if min >= max
 */
HPAPI int HP_RandRangeInt(HP_RandGen* generator, int min, int max);

/**
 * @brief Generate a random unsigned integer in a given range [min, max]
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random unsigned integer in range, or min if min >= max
 */
HPAPI uint32_t HP_RandRangeUint(HP_RandGen* generator, uint32_t min, uint32_t max);

/**
 * @brief Generate a random float in a given range [min, max)
 * @param generator Pointer to the generator to use, or NULL for the default generator
 * @param min Minimum value (inclusive)
 * @param max Maximum value (exclusive)
 * @return Random float in range
 */
HPAPI float HP_RandRangeFloat(HP_RandGen* generator, float min, float max);

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
HPAPI void HP_RandShuffle(HP_RandGen* generator, void* array, size_t element_size, size_t count);

/** @} */ // Random

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // HP_RAND_H
