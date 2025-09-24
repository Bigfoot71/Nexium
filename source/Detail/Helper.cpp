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

#include "./Helper.hpp"
#include <SDL3/SDL_stdinc.h>

namespace {
char gBuffer[256];
}

namespace helper {

const char* getFileExt(const char* filePath)
{
    const char* dot = strrchr(filePath, '.');
    if (!dot || dot == filePath) return NULL;
    return dot + 1;
}

const char* concatCString(const char* a, const char* b)
{
    gBuffer[0] = '\0';
    if (a) SDL_strlcat(gBuffer, a, sizeof(gBuffer));
    if (b) SDL_strlcat(gBuffer, b, sizeof(gBuffer));
    return gBuffer;
}

const char* formatCString(const char* fmt, ...)
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
