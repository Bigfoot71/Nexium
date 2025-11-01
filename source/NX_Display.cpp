/* NX_Display.cpp -- API definition for Nexium's display module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Display.h>

#include "./INX_GlobalState.hpp"
#include <SDL3/SDL_video.h>

// ============================================================================
// PUBLIC API
// ============================================================================

float NX_GetDisplayScale()
{
    return SDL_GetWindowDisplayScale(INX_Display.window);
}

float NX_GetDisplayGetDPI()
{
    float displayScale = SDL_GetWindowDisplayScale(INX_Display.window);

#if defined(__ANDROID__) || defined(__IPHONEOS__)
    return displayScale * 160.0f;
#else
    return displayScale * 96.0f;
#endif
}

int NX_GetDisplayIndex()
{
    return SDL_GetDisplayForWindow(INX_Display.window);
}

NX_IVec2 NX_GetDisplaySize()
{
    int displayIndex = SDL_GetDisplayForWindow(INX_Display.window);

    SDL_Rect bounds{};
    SDL_GetDisplayBounds(displayIndex, &bounds);

    return NX_IVEC2(bounds.w, bounds.h);
}

NX_Vec2 NX_GetDisplaySizeF()
{
    int displayIndex = SDL_GetDisplayForWindow(INX_Display.window);

    SDL_Rect bounds{};
    SDL_GetDisplayBounds(displayIndex, &bounds);

    return NX_VEC2(bounds.w, bounds.h);
}
