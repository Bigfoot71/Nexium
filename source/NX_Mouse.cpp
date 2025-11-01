/* NX_Mouse.c -- API definition for Nexium's mouse module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Mouse.h>
#include "./INX_GlobalState.hpp"

#include <SDL3/SDL_mouse.h>

// ============================================================================
// PUBLIC API
// ============================================================================

void NX_CaptureMouse(bool enabled)
{
    SDL_SetWindowRelativeMouseMode(INX_Display.window, enabled);
}

bool NX_IsMouseButtonPressed(NX_MouseButton buttons)
{
    return (INX_Mouse.buttons[0] & buttons);
}

bool NX_IsMouseButtonReleased(NX_MouseButton buttons)
{
    return !(INX_Mouse.buttons[0] & buttons);
}

bool NX_IsMouseButtonJustPressed(NX_MouseButton buttons)
{
    bool current = (INX_Mouse.buttons[0] & buttons);
    bool previous = (INX_Mouse.buttons[1] & buttons);

    return current && !previous;
}

bool NX_IsMouseButtonJustReleased(NX_MouseButton buttons)
{
    bool current = (INX_Mouse.buttons[0] & buttons);
    bool previous = (INX_Mouse.buttons[1] & buttons);

    return !current && previous;
}

NX_Vec2 NX_GetMousePosition()
{
    return INX_Mouse.position;
}

void NX_SetMousePosition(NX_Vec2 p)
{
    SDL_WarpMouseInWindow(INX_Display.window, p.x, p.y);
    INX_Mouse.position = p;
}

NX_Vec2 NX_GetMouseDelta()
{
    return INX_Mouse.delta;
}

NX_Vec2 NX_GetMouseWheel()
{
    return INX_Mouse.wheel;
}
