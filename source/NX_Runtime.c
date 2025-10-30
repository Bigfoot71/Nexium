/* NX_Runtime.c -- API definition for Nexium's frame module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Runtime.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_time.h>
#include "./INX_GlobalState.h"

// ============================================================================
// PUBLIC API
// ============================================================================

bool NX_FrameStep(void)
{
    bool shouldRun = true;

    /* --- Buffer swap --- */

    static bool firstFrame = true;

    if (!firstFrame) {
        SDL_GL_SwapWindow(INX_Display.window);
    }
    else {
        firstFrame = false;
    }

    /* --- Calculate delta time and sleep if enough time remains --- */

    Uint64 ticksNow = SDL_GetPerformanceCounter();
    INX_Frame.currentDeltaTime = (double)(ticksNow - INX_Frame.ticksLast) / INX_Frame.perfFrequency;

    const double sleepSafetyMargin = 0.002;
    if (INX_Frame.currentDeltaTime < INX_Frame.targetDeltaTime - sleepSafetyMargin) {
        SDL_DelayNS(1e9 * (Uint64)((INX_Frame.targetDeltaTime - INX_Frame.currentDeltaTime - sleepSafetyMargin)));
    }

    /* --- Get accurate delta time after sleep and busy-wait remaining time if needed --- */

    do {
        ticksNow = SDL_GetPerformanceCounter();
        INX_Frame.currentDeltaTime = (double)(ticksNow - INX_Frame.ticksLast) / INX_Frame.perfFrequency;
    } while (INX_Frame.currentDeltaTime < INX_Frame.targetDeltaTime);

    INX_Frame.elapsedTime += INX_Frame.currentDeltaTime;
    INX_Frame.ticksLast = ticksNow;

    /* --- FPS smoothing using exponential moving average --- */

    const double smoothingFactor = 0.1;
    double currentFPS = 1.0 / INX_Frame.currentDeltaTime;
    INX_Frame.fpsAverage = INX_Frame.fpsAverage * (1.0 - smoothingFactor) + currentFPS * smoothingFactor;

    /* --- Update input state --- */

    // Shift current >> previous state
    for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
        INX_Keyboard.keys[i] = (INX_Keyboard.keys[i] & 0xF0) | (INX_Keyboard.keys[i] >> 4);
    }

    INX_Mouse.buttons[1] = INX_Mouse.buttons[0];
    INX_Mouse.delta = NX_VEC2_ZERO;
    INX_Mouse.wheel = NX_VEC2_ZERO;

    /* --- Update system events --- */

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_EVENT_QUIT:
            shouldRun = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            INX_Keyboard.keys[ev.key.scancode] |= 0xF0;
            break;
        case SDL_EVENT_KEY_UP:
            INX_Keyboard.keys[ev.key.scancode] &= 0x0F;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            INX_Mouse.buttons[0] |= SDL_BUTTON_MASK(ev.button.button);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            INX_Mouse.buttons[0] &= ~SDL_BUTTON_MASK(ev.button.button);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            INX_Mouse.position.x = ev.motion.x;
            INX_Mouse.position.y = ev.motion.y;
            INX_Mouse.delta.x = ev.motion.xrel;
            INX_Mouse.delta.y = ev.motion.yrel;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            INX_Mouse.wheel.x = ev.wheel.x;
            INX_Mouse.wheel.y = ev.wheel.y;
            break;
        default:
            break;
        }
    }

    return shouldRun;
}

int64_t NX_GetCurrentTimeNS(void)
{
    SDL_Time time = 0;
    SDL_GetCurrentTime(&time);
    return time;
}

double NX_GetCurrentTime(void)
{
    int64_t ns = NX_GetCurrentTimeNS();
    return (double)(ns) / 1e9;
}

double NX_GetElapsedTime(void)
{
    return INX_Frame.elapsedTime;
}

double NX_GetDeltaTime(void)
{
    return INX_Frame.currentDeltaTime;
}

int NX_GetFPS(void)
{
    return (int)round(INX_Frame.fpsAverage);
}

void NX_SetTargetFPS(int fps)
{
    INX_Frame.targetDeltaTime = 1.0 / (double)(fps);
}

bool NX_SetVSync(int mode)
{
    return SDL_GL_SetSwapInterval(mode);
}
