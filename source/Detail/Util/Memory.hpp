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

template <typename T>
inline T* malloc(size_t count = 1)
{
    static_assert(!std::is_same_v<T, void>);

    return static_cast<T*>(SDL_malloc(count * sizeof(T)));
}

template <typename T>
inline T* calloc(size_t count = 1)
{
    static_assert(!std::is_same_v<T, void>);

    return static_cast<T*>(SDL_calloc(count, sizeof(T)));
}

template <typename T>
inline T* realloc(T* mem, size_t count)
{
    static_assert(!std::is_same_v<T, void>);

    return static_cast<T*>(SDL_realloc(mem, count * sizeof(T)));
}

inline void free(void* mem)
{
    return SDL_free(mem);
}

} // namespace mem

#endif // NX_UTIL_MEMORY_HPP
