/* Helper.cpp -- Contains a collection of various helpers
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./Helper.hpp"

namespace {
char gBuffer[256];
}

namespace helper {

const char* ConcatCStr(const char* a, const char* b)
{
    gBuffer[0] = '\0';
    if (a) SDL_strlcat(gBuffer, a, sizeof(gBuffer));
    if (b) SDL_strlcat(gBuffer, b, sizeof(gBuffer));
    return gBuffer;
}

const char* FormatCStr(const char* fmt, ...)
{
    gBuffer[0] = '\0';
    if (fmt) {
        va_list args;
        va_start(args, fmt);
        SDL_vsnprintf(gBuffer, sizeof(gBuffer), fmt, args);
        va_end(args);
    }
    return gBuffer;
}

} // namespace helper
