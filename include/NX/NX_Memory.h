/* NX_Memory.h -- API declaration for Nexium's memory module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MEMORY_H
#define NX_MEMORY_H

#include "./NX_API.h"
#include <stddef.h>

// ============================================================================
// FUNCTIONS DECLARATIONS
// ============================================================================

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Allocates a memory block of the given size.
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
NXAPI void* NX_Malloc(size_t size);

/**
 * @brief Allocates and zero-initializes an array.
 * @param nmemb Number of elements.
 * @param size Size of each element in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
NXAPI void* NX_Calloc(size_t nmemb, size_t size);

/**
 * @brief Resizes a previously allocated memory block.
 * @param ptr Pointer to the memory block to resize.
 * @param size New size in bytes.
 * @return Pointer to the reallocated memory, or NULL if allocation fails.
 */
NXAPI void* NX_Realloc(void* ptr, size_t size);

/**
 * @brief Frees a previously allocated memory block.
 * @param ptr Pointer to the memory block to free.
 */
NXAPI void NX_Free(void* ptr);

#if defined(__cplusplus)
} // extern "C"
#endif

// ============================================================================
// C++ Helpers
// ============================================================================

#if defined(__cplusplus)

#include <type_traits>

/**
 * @brief Allocates memory using NX_Malloc.
 * @tparam T Element type to allocate. If void, allocates raw bytes.
 */
template <typename T>
inline T* NX_Malloc(size_t count = 1)
{
    if constexpr (std::is_same_v<T, void>) {
        return NX_Malloc(count);
    }
    else {
        return static_cast<T*>(NX_Malloc(count * sizeof(T)));
    }
}

/**
 * @brief Allocates zero-initialized memory using NX_Calloc.
 * @tparam T Element type to allocate. If void, allocates raw bytes.
 */
template <typename T>
inline T* NX_Calloc(size_t count = 1)
{
    if constexpr (std::is_same_v<T, void>) {
        return NX_Calloc(count, 1);
    }
    else {
        return static_cast<T*>(NX_Calloc(count, sizeof(T)));
    }
}

/**
 * @brief Reallocates memory using NX_Realloc.
 * @tparam T Element type of the memory block. If void, uses raw byte size.
 */
template <typename T = void>
inline T* NX_Realloc(T* mem, size_t count)
{
    if constexpr (std::is_same_v<T, void>) {
        return NX_Realloc(mem, count);
    }
    else {
        return static_cast<T*>(NX_Realloc(mem, count * sizeof(T)));
    }
}

#endif // __cplusplus

#endif // NX_MEMORY_H
