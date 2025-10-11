/* Helper.hpp -- Contains a collection of various helpers
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_DETAIL_HELPER_HPP
#define NX_DETAIL_HELPER_HPP

#include <SDL3/SDL_stdinc.h>
#include <NX/NX_BitUtils.h>
#include <NX/NX_Macros.h>
#include <type_traits>

#if defined(_MSVC_VER)
#include <intrin.h>
#endif

namespace helper {

/**
 * @brief Find the index of the first set bit (least significant 1) in a 32-bit bitfield.
 * 
 * @param bitfield The bitfield to scan.
 * @return int Index of the first set bit, or -1 if no bits are set.
 * 
 * @note Uses constexpr iteration if evaluated at compile-time.
 *       Uses compiler intrinsics for fast runtime evaluation.
 */
constexpr int bitScanForward(uint32_t bitfield)
{
    if (std::is_constant_evaluated()) {
        for (int i = 0; i < 32; ++i) {
            if (bitfield & (1u << i)) return i;
        }
        return -1;
    }

    return (bitfield == 0) ? -1 : static_cast<int>(
        NX_Ctz64(static_cast<uint64_t>(bitfield))
    );
}

/**
 * @brief Find the index of the first set bit starting from the most significant bit in a 32-bit bitfield.
 * 
 * @param bitfield The bitfield to scan.
 * @return int Index of the most significant set bit, or -1 if no bits are set.
 * 
 * @note Uses constexpr iteration if evaluated at compile-time.
 *       Uses compiler intrinsics for fast runtime evaluation.
 */
constexpr int bitScanReverse(uint32_t bitfield)
{
    if (std::is_constant_evaluated()) {
        for (int i = 31; i >= 0; --i) {
            if (bitfield & (1u << i)) return i;
        }
        return -1;
    }

    return (bitfield == 0) ? -1 : static_cast<int>(
        NX_Clz64(static_cast<uint64_t>(bitfield))
    );
}

/**
 * @brief Iterate over all set bits in a bitfield and call a provided function on each index.
 * 
 * @tparam Func Callable type (e.g., lambda) accepting an int index.
 * @param bitfield The bitfield containing the flags to iterate over.
 * @param func The callable to invoke for each set bit index.
 */
template <typename Func>
inline void forEachBit(uint32_t bitfield, Func&& func)
{
    while (bitfield != 0) {
        int index = helper::bitScanForward(bitfield);
        func(index);
        bitfield &= ~(1u << index);
    }
}

/**
 * @brief Get the file extension from a file path.
 * 
 * @param filePath The path to the file as a null-terminated string.
 * @return const char* Pointer to the file extension, or nullptr if none exists.
 * 
 * @note The returned pointer points inside the original string.
 */
inline const char* getFileExt(const char* filePath)
{
    const char* dot = SDL_strrchr(filePath, '.');
    if (!dot || dot == filePath) return nullptr;
    return dot + 1;
}

/**
 * @brief Concatenate two C-style strings into a single string.
 * 
 * @param a First null-terminated string.
 * @param b Second null-terminated string.
 * @return const char* Pointer to the concatenated string.
 * 
 * @note The result is stored in a static buffer and must be used immediately.
 */
const char* concatCString(const char* a, const char* b);

/**
 * @brief Format a string using a printf-style format string and variadic arguments.
 * 
 * @param fmt The format string.
 * @param ... Variadic arguments to format into the string.
 * @return const char* Pointer to the formatted string.
 * 
 * @note The result is stored in a static buffer and must be consumed immediately.
 */
const char* formatCString(const char* fmt, ...);

} // namespace helper

#endif // NX_DETAIL_HELPER_HPP
