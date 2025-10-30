/* Memory.hpp -- Contains helpers for memory allocation based on SDL_stdinc.h
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_UTIL_MEMORY_HPP
#define NX_UTIL_MEMORY_HPP

#include <NX/NX_Memory.h>
#include <memory>

namespace util {

/**
 * @brief Custom deleter using NX_Free.
 */
template <typename T = void>
struct Deleter {
    void operator()(T* ptr) const noexcept {
        NX_Free(ptr);
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
    return UniquePtr<T>(NX_Malloc<T>(count));
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
    T* ptr = NX_Malloc<T>(count);
    return SharedPtr<T>(ptr, Deleter<T>{});
}

} // namespace util

#endif // NX_UTIL_MEMORY_HPP
