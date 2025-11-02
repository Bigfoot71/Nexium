/* INX_GlobalState.hpp -- Internal implementation details for managing global states
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef INX_GLOBAL_STATE_HPP
#define INX_GLOBAL_STATE_HPP

#include <NX/NX_Math.h>

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <alc.h>

// ============================================================================
// GLOBAL STATES
// ============================================================================

extern struct INX_DisplayState {
    SDL_Window* window;
    SDL_GLContext glContext;
    SDL_GLProfile glProfile;
} INX_Display;

extern struct INX_KeyboardState {
    Uint8 keys[SDL_SCANCODE_COUNT];     //< MSB = Current | LSB = Previous
} INX_Keyboard;

extern struct INX_MouseState {
    SDL_MouseButtonFlags buttons[2];    //< [0] = Current | [1] = Previous
    NX_Vec2 position;
    NX_Vec2 delta;
    NX_Vec2 wheel;
} INX_Mouse;

extern struct INX_FrameState {
    Uint64 ticksLast;
    Uint64 perfFrequency;
    double targetDeltaTime;
    double currentDeltaTime;
    double elapsedTime;
    double fpsAverage;
} INX_Frame;

#endif // INX_GLOBAL_STATE_HPP
