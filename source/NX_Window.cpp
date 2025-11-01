/* NX_Window.c -- API definition for Nexium's window module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Window.h>
#include <NX/NX_Log.h>

#include "./INX_GlobalState.hpp"

// ============================================================================
// PUBLIC API
// ============================================================================

const char* NX_GetWindowTitle()
{
    return SDL_GetWindowTitle(INX_Display.window);
}

void NX_SetWindowTitle(const char* title)
{
    SDL_SetWindowTitle(INX_Display.window, title);
}

void NX_SetWindowIcon(const NX_Image* icon)
{
    if (icon == NULL || icon->pixels == NULL) {
        NX_LOG(E, "CORE: Failed to set window icon; Invalid icon data");
        return;
    }

    SDL_PixelFormat format{};
    int bpp{};

    switch (icon->format) {
    case NX_PIXEL_FORMAT_RGB8:
        format = SDL_PIXELFORMAT_RGB24, bpp = 3;
        break;
    case NX_PIXEL_FORMAT_RGBA8:
        format = SDL_PIXELFORMAT_RGBA32, bpp = 4;
        break;
    case NX_PIXEL_FORMAT_RGB16F:
        format = SDL_PIXELFORMAT_RGB48_FLOAT, bpp = 6;
        break;
    case NX_PIXEL_FORMAT_RGBA16F:
        format = SDL_PIXELFORMAT_RGBA64_FLOAT, bpp = 8;
        break;
    case NX_PIXEL_FORMAT_RGB32F:
        format = SDL_PIXELFORMAT_RGB96_FLOAT, bpp = 12;
        break;
    case NX_PIXEL_FORMAT_RGBA32F:
        format = SDL_PIXELFORMAT_RGBA128_FLOAT, bpp = 16;
        break;
    default:
        NX_LOG(E, "CORE: Failed to set window icon; Unsupported format");
        return;
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(icon->w, icon->h, format, 
                                                 icon->pixels, bpp * icon->w);
    if (surface == NULL) {
        NX_LOG(E, "CORE: Failed to set window icon; %s", SDL_GetError());
        return;
    }

    if (!SDL_SetWindowIcon(INX_Display.window, surface)) {
        NX_LOG(E, "CORE: Failed to set window icon; %s", SDL_GetError());
    }

    SDL_DestroySurface(surface);
}

int NX_GetWindowWidth()
{
    int w = 0;
    SDL_GetWindowSize(INX_Display.window, &w, NULL);
    return w;
}

int NX_GetWindowHeight()
{
    int h = 0;
    SDL_GetWindowSize(INX_Display.window, NULL, &h);
    return h;
}

NX_IVec2 NX_GetWindowSize()
{
    NX_IVec2 result = NX_IVEC2_ZERO;
    SDL_GetWindowSize(INX_Display.window, &result.x, &result.y);
    return result;
}

NX_Vec2 NX_GetWindowSizeF()
{
    NX_IVec2 result = NX_IVEC2_ZERO;
    SDL_GetWindowSize(INX_Display.window, &result.x, &result.y);
    return NX_VEC2(result.x, result.y);
}

void NX_SetWindowSize(int w, int h)
{
    SDL_SetWindowSize(INX_Display.window, w, h);
}

void NX_SetWindowMinSize(int w, int h)
{
    SDL_SetWindowMinimumSize(INX_Display.window, w, h);
}

void NX_SetWindowMaxSize(int w, int h)
{
    SDL_SetWindowMaximumSize(INX_Display.window, w, h);
}

NX_IVec2 NX_GetWindowPosition()
{
    NX_IVec2 result = NX_IVEC2_ZERO;
    SDL_GetWindowPosition(INX_Display.window, &result.x, &result.y);
    return result;
}

void NX_SetWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(INX_Display.window, x, y);
}

bool NX_IsWindowFullscreen()
{
    Uint64 flags = SDL_GetWindowFlags(INX_Display.window);
    return (flags & SDL_WINDOW_FULLSCREEN) != 0;
}

void NX_SetWindowFullscreen(bool enabled)
{
    SDL_SetWindowFullscreen(INX_Display.window, enabled);
}

bool NX_IsWindowResizable()
{
    Uint64 flags = SDL_GetWindowFlags(INX_Display.window);
    return (flags & SDL_WINDOW_RESIZABLE) != 0;
}

void NX_SetWindowResizable(bool resizable)
{
    SDL_SetWindowResizable(INX_Display.window, resizable);
}

bool NX_IsWindowVisible()
{
    Uint64 flags = SDL_GetWindowFlags(INX_Display.window);
    return (flags & SDL_WINDOW_HIDDEN) == 0;
}

void NX_MinimizeWindow()
{
    SDL_MinimizeWindow(INX_Display.window);
}

void NX_MaximizeWindow()
{
    SDL_MaximizeWindow(INX_Display.window);
}

void NX_RestoreWindow()
{
    SDL_RestoreWindow(INX_Display.window);
}

void NX_ShowWindow()
{
    SDL_ShowWindow(INX_Display.window);
}

void NX_HideWindow()
{
    SDL_HideWindow(INX_Display.window);
}

bool NX_IsWindowFocused()
{
    Uint64 flags = SDL_GetWindowFlags(INX_Display.window);
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void NX_FocusWindow()
{
    SDL_RaiseWindow(INX_Display.window);
}

bool NX_IsWindowBordered()
{
    Uint64 flags = SDL_GetWindowFlags(INX_Display.window);
    return (flags & SDL_WINDOW_BORDERLESS) == 0;
}

void NX_SetWindowBordered(bool bordered)
{
    SDL_SetWindowBordered(INX_Display.window, bordered);
}

bool NX_IsCursorGrabbed()
{
    return SDL_GetWindowMouseGrab(INX_Display.window);
}

void NX_GrabCursor(bool grab)
{
    SDL_SetWindowMouseGrab(INX_Display.window, grab);
}

void NX_ShowCursor()
{
    SDL_ShowCursor();
}

void NX_HideCursor()
{
    SDL_HideCursor();
}

bool NX_IsCursorVisible()
{
    return SDL_CursorVisible();
}
