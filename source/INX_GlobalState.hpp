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
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <alc.h>

#include <array>

// ============================================================================
// GLOBAL STATES
// ============================================================================

extern struct INX_DisplayState {
    SDL_Window* window{};
    SDL_GLContext glContext{};
    SDL_GLProfile glProfile{};
} INX_Display;

extern struct INX_KeyboardState {
    std::array<Uint8, SDL_SCANCODE_COUNT> keys{};   //< MSB = Current | LSB = Previous
} INX_Keyboard;

extern struct INX_MouseState {
    std::array<SDL_MouseButtonFlags, 2> buttons{};  //< [0] = Current | [1] = Previous
    NX_Vec2 position{};
    NX_Vec2 delta{};
    NX_Vec2 wheel{};
} INX_Mouse;

extern struct INX_GamepadState {
    struct Device {
        SDL_JoystickID id{};
        SDL_Gamepad* gamepad{};
        std::array<Uint8, SDL_GAMEPAD_BUTTON_COUNT> buttons{};  //< MSB = Current | LSB = Previous
    };
    static constexpr int MaxGampads = 4;
    std::array<Device, MaxGampads> devices{};
} INX_Gamepad;

extern struct INX_FrameState {
    Uint64 ticksLast{};
    Uint64 perfFrequency{};
    double targetDeltaTime{};
    double currentDeltaTime{};
    double elapsedTime{};
    double fpsAverage{};
} INX_Frame;

#endif // INX_GLOBAL_STATE_HPP
