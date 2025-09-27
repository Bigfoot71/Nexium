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

#ifndef HP_DETAIL_HELPER_HPP
#define HP_DETAIL_HELPER_HPP

#include <Hyperion/HP_BitUtils.h>
#include <Hyperion/HP_Macros.h>
#include <SDL3/SDL_stdinc.h>
#include <type_traits>

#if defined(_MSVC_VER)
#include <intrin.h>
#endif

namespace helper {

/**
 * @brief Find the index of the first set bit (least significant 1) in a 32-bit flag.
 * 
 * @param flag The bitfield to scan.
 * @return int Index of the first set bit, or -1 if no bits are set.
 * 
 * @note Uses constexpr iteration if evaluated at compile-time.
 *       Uses compiler intrinsics for fast runtime evaluation.
 */
constexpr int bitScanForward(uint32_t flag)
{
    if (std::is_constant_evaluated()) {
        for (int i = 0; i < 32; ++i) {
            if (flag & (1u << i)) return i;
        }
        return -1;
    }

    return (flag == 0) ? -1 : static_cast<int>(
        HP_Ctz64(static_cast<uint64_t>(flag))
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

#endif // HP_DETAIL_HELPER_HPP
