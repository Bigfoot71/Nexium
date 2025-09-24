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

#ifndef HP_CODEPOINT_H
#define HP_CODEPOINT_H

#include "./HP_Platform.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @defgroup Codepoint Unicode Codepoint Operations
 * @brief Functions for handling Unicode codepoints and UTF-8 conversion.
 * @{
 */

/**
 * @brief Gets the next Unicode codepoint from UTF-8 text.
 * @param text UTF-8 encoded text
 * @param codepointSize Output parameter for the size in bytes of the codepoint
 * @return Unicode codepoint value
 */
HPAPI int HP_GetCodepointNext(const char* text, int* codepointSize);

/**
 * @brief Gets the previous Unicode codepoint from UTF-8 text.
 * @param text UTF-8 encoded text
 * @param codepointSize Output parameter for the size in bytes of the codepoint
 * @return Unicode codepoint value
 */
HPAPI int HP_GetCodepointPrev(const char* text, int* codepointSize);

/**
 * @brief Counts the total number of Unicode codepoints in UTF-8 text.
 * @param text UTF-8 encoded text
 * @return Number of codepoints
 */
HPAPI int HP_GetCodepointCount(const char* text);

/**
 * @brief Converts a Unicode codepoint to UTF-8 encoding.
 * @param codepoint Unicode codepoint value
 * @param utf8Size Output parameter for the UTF-8 byte size
 * @return UTF-8 encoded string
 */
HPAPI const char* HP_CodepointToUTF8(int codepoint, int* utf8Size);

/**
 * @brief Extracts a Unicode codepoint from UTF-8 text.
 * @param text UTF-8 encoded text
 * @param codepointSize Output parameter for the size in bytes of the codepoint
 * @return Unicode codepoint value
 */
HPAPI int HP_GetCodepointFromUTF8(const char* text, int* codepointSize);

/**
 * @brief Converts an array of codepoints to UTF-8 string.
 * @param codepoints Array of Unicode codepoint values
 * @param length Number of codepoints in the array
 * @return Allocated UTF-8 encoded string
 */
HPAPI char* HP_ConvertCodepointsToUTF8(const int* codepoints, int length);

/**
 * @brief Converts UTF-8 string to array of codepoints.
 * @param text UTF-8 encoded text
 * @param count Output parameter for the number of codepoints
 * @return Allocated array of Unicode codepoint values
 */
HPAPI int* HP_ConvertCodepointsFromUTF8(const char* text, int* count);

/** @} */

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // HP_CODEPOINT_H
