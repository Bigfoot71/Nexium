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
 * @brief std::unique_ptr with util::Deleter.
 */
template <typename T>
using UniquePtr = std::unique_ptr<T, Deleter<T>>;

/**
 * @brief Allocates and constructs a single object.
 */
template <typename T, typename... Args>
inline UniquePtr<T> MakeUnique(Args&&... args)
{
    T* ptr = NX_Malloc<T>(1);
    new (ptr) T(std::forward<Args>(args)...);
    return UniquePtr<T>(ptr);
}

/**
 * @brief Allocates and constructs an array of objects.
 */
template <typename T>
inline UniquePtr<T> MakeUniqueArray(size_t count = 1)
{
    T* ptr = NX_Malloc<T>(count);
    for (size_t i = 0; i < count; ++i) {
        new (ptr + i) T();
    }
    return UniquePtr<T>(ptr);
}

/**
 * @brief std::shared_ptr with util::Deleter.
 */
template <typename T>
using SharedPtr = std::shared_ptr<T>;

/**
 * @brief Allocates and constructs a single object, returning a SharedPtr.
 */
template <typename T, typename... Args>
inline SharedPtr<T> MakeShared(Args&&... args)
{
    T* ptr = NX_Malloc<T>(1);
    new (ptr) T(std::forward<Args>(args)...);
    return SharedPtr<T>(ptr, Deleter<T>{});
}

/**
 * @brief Allocates and constructs an array of objects, returning a SharedPtr.
 */
template <typename T>
inline SharedPtr<T> MakeSharedArray(size_t count = 1)
{
    T* ptr = NX_Malloc<T>(count);
    for (size_t i = 0; i < count; ++i) {
        new (ptr + i) T();
    }
    return SharedPtr<T>(ptr, Deleter<T>{});
}

} // namespace util

#endif // NX_UTIL_MEMORY_HPP
