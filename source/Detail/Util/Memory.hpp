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

namespace util {

template <typename T = void>
inline T* malloc(size_t count = 1)
{
    if constexpr (std::is_same_v<T, void>) {
        return SDL_malloc(count);
    }

    return static_cast<T*>(SDL_malloc(count * sizeof(T)));
}

template <typename T = void>
inline T* calloc(size_t count = 1)
{
    if constexpr (std::is_same_v<T, void>) {
        return SDL_calloc(count, 1);
    }

    return static_cast<T*>(SDL_calloc(count, sizeof(T)));
}

inline void* calloc(size_t count, size_t size)
{
    return SDL_calloc(count, size);
}

template <typename T = void>
inline T* realloc(T* mem, size_t count)
{
    if constexpr (std::is_same_v<T, void>) {
        return SDL_realloc(mem, count);
    }

    return static_cast<T*>(SDL_realloc(mem, count * sizeof(T)));
}

inline void free(void* mem)
{
    return SDL_free(mem);
}

} // namespace mem

#endif // NX_UTIL_MEMORY_HPP
