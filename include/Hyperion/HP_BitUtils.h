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

#ifndef HP_BIT_UTILS_H
#define HP_BIT_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#if defined(_MSC_VER)
#   include <intrin.h>
#endif

/* === Macros === */

/**
 * @brief Sets a specific bit in a variable
 */
#define HP_BIT_SET(var, bit) \
    ((var) |= (1ULL << (bit)))

/**
 * @brief Clears a specific bit in a variable
 */
#define HP_BIT_CLEAR(var, bit) \
    ((var) &= ~(1ULL << (bit)))

/**
 * @brief Toggles a specific bit in a variable
 */
#define HP_BIT_TOGGLE(var, bit) \
    ((var) ^= (1ULL << (bit)))

/**
 * @brief Checks if a specific bit is set
 */
#define HP_BIT_CHECK(var, bit) \
    (!!((var) & (1ULL << (bit))))

/* === Functions === */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Counts the number of leading zeros in a 64-bit integer
 */
static inline int HP_Clz64(uint64_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return (x) ? __builtin_clzll(x) : 64;
#elif defined(_MSC_VER)
    if (x == 0) return 64;
    unsigned long i;
    _BitScanReverse64(&i, x);
	return 63 - (int)i;
#else
    if (x == 0) return 64;
    int n = 0;
    while ((x & (1ULL << 63)) == 0) { n++; x <<= 1; }
    return n;
#endif
}

/**
 * @brief Counts the number of trailing zeros in a 64-bit integer
 */
static inline int HP_Ctz64(uint64_t x)
{
#if defined(__GNUC__) || defined(__clang__)
	return (x) ? __builtin_ctzll(x) : 64;
#elif defined(_MSC_VER)
    if (x == 0) return 64;
    unsigned long i;
	_BitScanForward64(&i, x);
    return (int)i;
#else
    if (x == 0) return 64;
    int n = 0;
    while ((x & 1) == 0) { n++; x >>= 1; }
    return n;
#endif
}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // HP_BIT_UTILS_H
