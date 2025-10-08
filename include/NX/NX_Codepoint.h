/* NX_Codepoint.h -- API declaration for Nexium's codepoint module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CODEPOINT_H
#define NX_CODEPOINT_H

#include "./NX_Platform.h"

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
NXAPI int NX_GetCodepointNext(const char* text, int* codepointSize);

/**
 * @brief Gets the previous Unicode codepoint from UTF-8 text.
 * @param text UTF-8 encoded text
 * @param codepointSize Output parameter for the size in bytes of the codepoint
 * @return Unicode codepoint value
 */
NXAPI int NX_GetCodepointPrev(const char* text, int* codepointSize);

/**
 * @brief Counts the total number of Unicode codepoints in UTF-8 text.
 * @param text UTF-8 encoded text
 * @return Number of codepoints
 */
NXAPI int NX_GetCodepointCount(const char* text);

/**
 * @brief Converts a Unicode codepoint to UTF-8 encoding.
 * @param codepoint Unicode codepoint value
 * @param utf8Size Output parameter for the UTF-8 byte size
 * @return UTF-8 encoded string
 */
NXAPI const char* NX_CodepointToUTF8(int codepoint, int* utf8Size);

/**
 * @brief Extracts a Unicode codepoint from UTF-8 text.
 * @param text UTF-8 encoded text
 * @param codepointSize Output parameter for the size in bytes of the codepoint
 * @return Unicode codepoint value
 */
NXAPI int NX_GetCodepointFromUTF8(const char* text, int* codepointSize);

/**
 * @brief Converts an array of codepoints to UTF-8 string.
 * @param codepoints Array of Unicode codepoint values
 * @param length Number of codepoints in the array
 * @return Allocated UTF-8 encoded string
 */
NXAPI char* NX_ConvertCodepointsToUTF8(const int* codepoints, int length);

/**
 * @brief Converts UTF-8 string to array of codepoints.
 * @param text UTF-8 encoded text
 * @param count Output parameter for the number of codepoints
 * @return Allocated array of Unicode codepoint values
 */
NXAPI int* NX_ConvertCodepointsFromUTF8(const char* text, int* count);

/** @} */

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // NX_CODEPOINT_H
