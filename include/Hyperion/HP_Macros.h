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

#ifndef HP_MACROS_H
#define HP_MACROS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Returns the minimum of two values
 */
#define HP_MIN(a, b) \
    ((a) < (b) ? (a) : (b))

/**
 * @brief Returns the maximum of two values
 */
#define HP_MAX(a, b) \
    ((a) > (b) ? (a) : (b))

/**
 * @brief Clamps a value between minimum and maximum bounds
 */
#define HP_CLAMP(v, min, max) \
    HP_MIN(HP_MAX((v), (min)), (max))

/**
 * @brief Returns the minimum of three values
 */
#define HP_MIN3(a, b, c) \
    HP_MIN(a, HP_MIN(b, c))

/**
 * @brief Returns the maximum of three values
 */
#define HP_MAX3(a, b, c) \
    HP_MAX(a, HP_MAX(b, c))

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
 * @brief Returns the absolute value of a number
 */
#define HP_ABS(x) \
    ((x) < 0 ? -(x) : (x))

/**
 * @brief Returns the sign of a number (-1, 0, or 1)
 */
#define HP_SIGN(x) \
    (((x) > 0) - ((x) < 0))

/**
 * @brief Returns the square of a number
 */
#define HP_POW2(x) \
    ((x) * (x))

/**
 * @brief Returns the cube of a number
 */
#define HP_POW3(x) \
    ((x) * (x) * (x))

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

/**
 * @brief Rounds value up to the next alignment boundary
 */
#define HP_ALIGN_UP(value, alignment) \
    (((value) + (alignment) - 1) & ~((alignment) - 1))

/**
 * @brief Rounds value down to the previous alignment boundary
 */
#define HP_ALIGN_DOWN(value, alignment) \
    ((value) & ~((alignment) - 1))

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
 * @brief Gets the offset of a member within a struct
 */
#define HP_OFFSET_OF(type, member) \
    offsetof(type, member)

/**
 * @brief Gets container struct from member pointer
 */
#define HP_CONTAINER_OF(ptr, type, member) \
    ((type*)((char*)(ptr) - HP_OFFSET_OF(type, member)))

/**
 * @brief Checks if value is within inclusive range
 */
#define HP_IN_RANGE(x, low, high) \
    ((x) >= (low) && (x) <= (high))

/**
 * @brief Checks if value is a power of 2
 */
#define HP_IS_PO2(x) \
    ((x) > 0 && ((x) & ((x) - 1)) == 0)

/**
 * @brief Next power of 2 (generic for uint8_t, uint16_t, uint32_t, uint64_t)
 */
#define HP_NEXT_PO2(x) \
    _Generic((x), \
        uint8_t: ({ \
            uint8_t _x = (x); \
            if (_x == 0) _x = 1; \
            else if ((_x & (_x - 1)) == 0) _x <<= 1; \
            else { \
                _x--; \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x++; \
            } \
            _x; \
        }), \
        uint16_t: ({ \
            uint16_t _x = (x); \
            if (_x == 0) _x = 1; \
            else if ((_x & (_x - 1)) == 0) _x <<= 1; \
            else { \
                _x--; \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x |= _x >> 8; \
                _x++; \
            } \
            _x; \
        }), \
        uint32_t: ({ \
            uint32_t _x = (x); \
            if (_x == 0) _x = 1; \
            else if ((_x & (_x - 1)) == 0) _x <<= 1; \
            else { \
                _x--; \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x |= _x >> 8; \
                _x |= _x >> 16; \
                _x++; \
            } \
            _x; \
        }), \
        uint64_t: ({ \
            uint64_t _x = (x); \
            if (_x == 0) _x = 1; \
            else if ((_x & (_x - 1)) == 0) _x <<= 1; \
            else { \
                _x--; \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x |= _x >> 8; \
                _x |= _x >> 16; \
                _x |= _x >> 32; \
                _x++; \
            } \
            _x; \
        }) \
    )

/**
 * @brief Previous power of 2 (generic for uint8_t, uint16_t, uint32_t, uint64_t)
 */
#define HP_PREV_PO2(x) \
    _Generic((x), \
        uint8_t: ({ \
            uint8_t _x = (x); \
            if (_x == 0) _x = 0; \
            else if ((_x & (_x - 1)) == 0) _x >>= 1; \
            else { \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x = _x - (_x >> 1); \
            } \
            _x; \
        }), \
        uint16_t: ({ \
            uint16_t _x = (x); \
            if (_x == 0) _x = 0; \
            else if ((_x & (_x - 1)) == 0) _x >>= 1; \
            else { \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x |= _x >> 8; \
                _x = _x - (_x >> 1); \
            } \
            _x; \
        }), \
        uint32_t: ({ \
            uint32_t _x = (x); \
            if (_x == 0) _x = 0; \
            else if ((_x & (_x - 1)) == 0) _x >>= 1; \
            else { \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x |= _x >> 8; \
                _x |= _x >> 16; \
                _x = _x - (_x >> 1); \
            } \
            _x; \
        }), \
        uint64_t: ({ \
            uint64_t _x = (x); \
            if (_x == 0) _x = 0; \
            else if ((_x & (_x - 1)) == 0) _x >>= 1; \
            else { \
                _x |= _x >> 1; \
                _x |= _x >> 2; \
                _x |= _x >> 4; \
                _x |= _x >> 8; \
                _x |= _x >> 16; \
                _x |= _x >> 32; \
                _x = _x - (_x >> 1); \
            } \
            _x; \
        }) \
    )

/**
 * @brief Nearest power of 2 (generic for uint8_t, uint16_t, uint32_t, uint64_t)
 */
#define HP_NEAR_PO2(x) \
    _Generic((x), \
        uint8_t: ({ \
            uint8_t _x = (x); \
            uint8_t _next = HP_NEXT_PO2(_x); \
            uint8_t _prev = HP_PREV_PO2(_x); \
            (_x - _prev < _next - _x) ? _prev : _next; \
        }), \
        uint16_t: ({ \
            uint16_t _x = (x); \
            uint16_t _next = HP_NEXT_PO2(_x); \
            uint16_t _prev = HP_PREV_PO2(_x); \
            (_x - _prev < _next - _x) ? _prev : _next; \
        }), \
        uint32_t: ({ \
            uint32_t _x = (x); \
            uint32_t _next = HP_NEXT_PO2(_x); \
            uint32_t _prev = HP_PREV_PO2(_x); \
            (_x - _prev < _next - _x) ? _prev : _next; \
        }), \
        uint64_t: ({ \
            uint64_t _x = (x); \
            uint64_t _next = HP_NEXT_PO2(_x); \
            uint64_t _prev = HP_PREV_PO2(_x); \
            (_x - _prev < _next - _x) ? _prev : _next; \
        }) \
    )

/**
 * @brief The multiple of B after A
 */
#define HP_NEXT_MULTIPLE(a, b) \
    ((b) * ((int)ceilf((float)(a) / (b))))

/**
 * @brief The multiple of B before A
 */
#define HP_PREV_MULTIPLE(a, b) \
    ((b) * ((int)floorf((float)(a) / (b))))

/**
 * @brief The closest multiple of B to A
 */
#define HP_NEAR_MULTIPLE(a, b) \
    ((b) * ((int)roundf((float)(a) / (b))))

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

/**
 * @brief Iterates over static array elements
 */
#define HP_FOR_EACH(item, array) \
    for (size_t _i = 0; _i < HP_ARRAY_SIZE(array) && ((item) = (array)[_i], 1); ++_i)

/**
 * @brief Integer division with ceiling (round up)
 */
#define HP_DIV_CEIL(num, denom) \
    (((num) + (denom) - 1) / (denom))

/**
 * @brief Checks if addition would overflow
 */
#define HP_WOULD_OVERFLOW_ADD(a, b, max) \
    ((a) > (max) - (b))

/**
 * @brief Checks if multiplication would overflow
 */
#define HP_WOULD_OVERFLOW_MUL(a, b, max) \
    ((a) != 0 && (b) > (max) / (a))

#endif // HP_MACROS_H
