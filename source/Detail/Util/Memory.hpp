/* Memory.hpp -- Contains helpers for memory allocation based on SDL_stdinc.h
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_MEMORY_HPP
#define NX_UTIL_MEMORY_HPP

#include <SDL3/SDL_stdinc.h>
#include <type_traits>
#include <memory>

namespace util {

/**
 * @brief Allocates memory for a given number of elements.
 */
template <typename T = void>
inline T* malloc(size_t count = 1)
{
    if constexpr (std::is_same_v<T, void>) {
        return SDL_malloc(count);
    }

    return static_cast<T*>(SDL_malloc(count * sizeof(T)));
}

/**
 * @brief Allocates and zero-initializes memory for a given number of elements.
 */
template <typename T = void>
inline T* calloc(size_t count = 1)
{
    if constexpr (std::is_same_v<T, void>) {
        return SDL_calloc(count, 1);
    }

    return static_cast<T*>(SDL_calloc(count, sizeof(T)));
}

/**
 * @brief Allocates and zero-initializes memory with explicit element size.
 */
inline void* calloc(size_t count, size_t size)
{
    return SDL_calloc(count, size);
}

/**
 * @brief Reallocates previously allocated memory to a new size.
 */
template <typename T = void>
inline T* realloc(T* mem, size_t count)
{
    if constexpr (std::is_same_v<T, void>) {
        return SDL_realloc(mem, count);
    }

    return static_cast<T*>(SDL_realloc(mem, count * sizeof(T)));
}

/**
 * @brief Frees previously allocated memory.
 */
inline void free(void* mem)
{
    return SDL_free(mem);
}

/**
 * @brief Custom deleter using util::free.
 */
template <typename T = void>
struct Deleter {
    void operator()(T* ptr) const noexcept {
        util::free(ptr);
    }
};

/**
 * @brief Alias for std::unique_ptr using util::Deleter.
 */
template <typename T>
using UniquePtr = std::unique_ptr<T, Deleter<T>>;

/**
 * @brief Allocates memory and returns a util::UniquePtr.
 */
template <typename T>
inline UniquePtr<T> makeUnique(size_t count = 1)
{
    return UniquePtr<T>(util::malloc<T>(count));
}

/**
 * @brief Alias for std::shared_ptr.
 */
template <typename T>
using SharedPtr = std::shared_ptr<T>;

/**
 * @brief Allocates memory and returns a util::SharedPtr using util::Deleter.
 */
template <typename T>
inline SharedPtr<T> makeShared(size_t count = 1)
{
    T* ptr = util::malloc<T>(count);
    return SharedPtr<T>(ptr, Deleter<T>{});
}

} // namespace util

#endif // NX_UTIL_MEMORY_HPP
