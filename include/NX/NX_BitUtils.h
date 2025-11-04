/* NX_BitUtils.h -- API declaration for Nexium's bit utils module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_BIT_UTILS_H
#define NX_BIT_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#if defined(_MSC_VER)
#   include <intrin.h>
#endif

/* === Macros === */

/**
 * @brief Sets a specific bit in a variable
 */
#define NX_BIT_SET(var, bit) \
    ((var) |= (1ULL << (bit)))

/**
 * @brief Clears a specific bit in a variable
 */
#define NX_BIT_CLEAR(var, bit) \
    ((var) &= ~(1ULL << (bit)))

/**
 * @brief Toggles a specific bit in a variable
 */
#define NX_BIT_TOGGLE(var, bit) \
    ((var) ^= (1ULL << (bit)))

/**
 * @brief Checks if a specific bit is set
 */
#define NX_BIT_CHECK(var, bit) \
    (!!((var) & (1ULL << (bit))))

/**
 * @brief Sets a specific flag in a variable
 */
#define NX_FLAG_SET(var, flag) \
    ((var) |= (flag))

/**
 * @brief Clears a specific flag in a variable
 */
#define NX_FLAG_CLEAR(var, flag) \
    ((var) &= ~(flag))

/**
 * @brief Toggles a specific flag in a variable
 */
#define NX_FLAG_TOGGLE(var, flag) \
    ((var) ^= (flag))

/**
 * @brief Checks if a specific flag is set
 */
#define NX_FLAG_CHECK(var, flag) \
    (!!((var) & (flag)))

/* === Functions === */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Counts the number of leading zeros in a 64-bit integer
 */
static inline int NX_Clz64(uint64_t x)
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
static inline int NX_Ctz64(uint64_t x)
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

#endif // NX_BIT_UTILS_H
