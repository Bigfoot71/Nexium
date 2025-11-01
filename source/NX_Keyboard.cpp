/* NX_Keyboard.c -- API definition for Nexium's keyboard module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Keyboard.h>
#include "./INX_GlobalState.hpp"

#include <SDL3/SDL_keyboard.h>

// ============================================================================
// PUBLIC API
// ============================================================================

bool NX_IsKeyPressed(NX_Key key)
{
    if (static_cast<Uint32>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }

    return (INX_Keyboard.keys[key] & 0xF0);
}

bool NX_IsKeyReleased(NX_Key key)
{
    if (static_cast<Uint32>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }

    return !(INX_Keyboard.keys[key] & 0xF0);
}

bool NX_IsKeyJustPressed(NX_Key key)
{
    if (static_cast<Uint32>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }

    bool current = (INX_Keyboard.keys[key] & 0xF0);
    bool previous = (INX_Keyboard.keys[key] & 0x0F);

    return (current && !previous);
}

bool NX_IsKeyJustReleased(NX_Key key)
{
    if (static_cast<Uint32>(key) >= SDL_SCANCODE_COUNT) {
        return false;
    }

    bool current = (INX_Keyboard.keys[key] & 0xF0);
    bool previous = (INX_Keyboard.keys[key] & 0x0F);

    return (!current && previous);
}

NX_Vec2 NX_GetKeyVec2(NX_Key up, NX_Key down, NX_Key left, NX_Key right)
{
    int x = NX_IsKeyPressed(right) - NX_IsKeyPressed(left);
    int y = NX_IsKeyPressed(down) - NX_IsKeyPressed(up);

    return NX_Vec2Normalize(NX_VEC2(x, y));
}

NX_Vec3 NX_GetKeyVec3(NX_Key forward, NX_Key backward, NX_Key left, NX_Key right)
{
    int x = NX_IsKeyPressed(right) - NX_IsKeyPressed(left);
    int z = NX_IsKeyPressed(backward) - NX_IsKeyPressed(forward);

    return NX_Vec3Normalize(NX_VEC3(x, 0, z));
}
