/* INX_Utils.cpp -- Contains a collection of various helpers
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./INX_Utils.hpp"

static char INX_CommonBuffer[256];

const char* INX_ConcatCString(const char* a, const char* b)
{
    INX_CommonBuffer[0] = '\0';
    if (a) SDL_strlcat(INX_CommonBuffer, a, sizeof(INX_CommonBuffer));
    if (b) SDL_strlcat(INX_CommonBuffer, b, sizeof(INX_CommonBuffer));
    return INX_CommonBuffer;
}

const char* INX_FormatCString(const char* fmt, ...)
{
    INX_CommonBuffer[0] = '\0';
    if (fmt) {
        va_list args;
        va_start(args, fmt);
        SDL_vsnprintf(INX_CommonBuffer, sizeof(INX_CommonBuffer), fmt, args);
        va_end(args);
    }
    return INX_CommonBuffer;
}
