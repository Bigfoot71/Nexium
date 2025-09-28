/* HP_Macros.h -- Contains help macros used internally but can be reused
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_MACROS_H
#define HP_MACROS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Cast macro that works in both C and C++
 */
#if defined(__cplusplus)
#   define HP_CAST(t, x) static_cast<t>(x)
#else
#   define HP_CAST(t, x) (t)(x)
#endif

/**
 * @brief Literal macro that works in both C and C++
 */
#if defined(__cplusplus)
#   define HP_LITERAL(type) type
#else
#   define HP_LITERAL(type) (type)
#endif

/**
 * @brief Struct literal macro that works in both C and C++
 */
#define HP_STRUCT_LITERAL(type, ...) \
    HP_LITERAL(type) {__VA_ARGS__}

/**
 * @brief Swaps two variables of compatible types
 */
#define HP_SWAP(a, b) \
    do { \
        __auto_type _temp = (a); \
        (a) = (b); \
        (b) = _temp; \
    } while(0)

/**
 * @brief Checks if a pointer is aligned to a boundary
 */
#define HP_IS_ALIGNED(ptr, alignment) \
    (((uintptr_t)(ptr) & ((alignment) - 1)) == 0)

/**
 * @brief Gets the number of elements in a static array
 */
#define HP_ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief Iterates over static array elements
 */
#define HP_FOR_EACH(item, array) \
    for (size_t _i = 0; _i < HP_ARRAY_SIZE(array) && ((item) = (array)[_i], 1); ++_i)

/**
 * @brief Gets container struct from member pointer
 */
#define HP_CONTAINER_OF(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

/**
 * @brief Suppresses unused variable warnings
 */
#define HP_UNUSED(x) ((void)(x))

/**
 * @brief Debug assertion
 */
#ifndef NDEBUG
#    define HP_ASSERT(cond)                                                    \
        do {                                                                   \
            if (!(cond)) {                                                     \
                fprintf(stderr,                                                \
                        "[ASSERT FAILED] %s\nFile: %s\nLine: %d\n",            \
                        #cond, __FILE__, __LINE__);                            \
                abort();                                                       \
            }                                                                  \
        } while (0)
#else
#    define HP_ASSERT(cond) ((void)0)
#endif

#endif // HP_MACROS_H
