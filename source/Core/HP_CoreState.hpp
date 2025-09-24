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

#ifndef HP_CORE_STATE_HPP
#define HP_CORE_STATE_HPP

#include <Hyperion/HP_Init.h>
#include <Hyperion/HP_Core.h>
#include <Hyperion/HP_Math.h>

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

#include <memory>

/* === Glabal State === */

extern std::unique_ptr<class HP_CoreState> gCore;

/* === Declaration === */

class HP_CoreState {
private:
    friend bool ::HP_InitEx(const char* title, int w, int h, HP_AppDesc* desc);
    friend void ::HP_SetMousePosition(HP_Vec2 p);
    friend bool ::HP_FrameStep(void);

public:
    /** Core data */
    SDL_Window* window() const;

    /** Framerate and time */
    void setTargetFrameRate(int fps) const;
    double elapsedTime() const;
    double frameTime() const;
    double frameRate() const;

    /** Input control */
    bool currentKey(HP_Key key) const;
    bool previousKey(HP_Key key) const;
    bool currentMouseButton(HP_MouseButton buttons);
    bool previousMouseButton(HP_MouseButton buttons);
    const HP_Vec2& mousePosition() const;
    const HP_Vec2& mouseDelta() const;
    const HP_Vec2& mouseWheel() const;

    /** OpenGL infos */
    SDL_GLProfile glProfile() const;

public:
    HP_CoreState(const char* title, int w, int h, const HP_AppDesc& desc);
    ~HP_CoreState();

private:
    static SDL_WindowFlags sdlWindowFlags(HP_Flags flags);

private:
    /** Core data */
    SDL_Window* mWindow{};
    SDL_GLContext mGLContext{};

    /** Framerate and time */
    Uint64 mTicksLast{};
    Uint64 mPerfFrequency{};
    double mTargetFrameTime{};
    double mCurrentFrameTime{};
    double mElapsedTime{};
    double mFpsAverage{};

    /** Input control */
    Uint8 mKeys[SDL_SCANCODE_COUNT]{};       //< MSB = Current | LSB = Previous
    SDL_MouseButtonFlags mMouseButtons[2]{}; //< [0] = Current | [1] = Previous
    HP_Vec2 mMousePosition{};
    HP_Vec2 mMouseDelta{};
    HP_Vec2 mMouseWheel{};

    /** OpenGL infos */
    SDL_GLProfile mGLProfile{};
};

/* === Public Implementation === */

inline SDL_Window* HP_CoreState::window() const
{
    return mWindow;
}

inline void HP_CoreState::setTargetFrameRate(int fps) const
{
    gCore->mTargetFrameTime = 1.0 / static_cast<double>(fps);
}

inline double HP_CoreState::elapsedTime() const
{
    return mElapsedTime;
}

inline double HP_CoreState::frameTime() const
{
    return mCurrentFrameTime;
}

inline double HP_CoreState::frameRate() const
{
    return mFpsAverage;
}

inline SDL_GLProfile HP_CoreState::glProfile() const
{
    return mGLProfile;
}

inline bool HP_CoreState::currentKey(HP_Key key) const
{
    SDL_assert(static_cast<uint32_t>(key) < SDL_SCANCODE_COUNT);

    return mKeys[key] & 0xF0;
}

inline bool HP_CoreState::previousKey(HP_Key key) const
{
    SDL_assert(static_cast<uint32_t>(key) < SDL_SCANCODE_COUNT);

    return mKeys[key] & 0x0F;
}

inline bool HP_CoreState::currentMouseButton(HP_MouseButton buttons)
{
    return (gCore->mMouseButtons[0] & buttons);
}

inline bool HP_CoreState::previousMouseButton(HP_MouseButton buttons)
{
    return (gCore->mMouseButtons[1] & buttons);
}

inline const HP_Vec2& HP_CoreState::mousePosition() const
{
    return mMousePosition;
}

inline const HP_Vec2& HP_CoreState::mouseDelta() const
{
    return mMouseDelta;
}

inline const HP_Vec2& HP_CoreState::mouseWheel() const
{
    return mMouseWheel;
}

/* === Private Implementation === */

inline SDL_WindowFlags HP_CoreState::sdlWindowFlags(HP_Flags flags)
{
    SDL_WindowFlags windowFlags = 0;
    if (flags & HP_FLAG_FULLSCREEN) windowFlags |= SDL_WINDOW_FULLSCREEN;
    if (flags & HP_FLAG_WINDOW_OCCLUDED) windowFlags |= SDL_WINDOW_OCCLUDED;
    if (flags & HP_FLAG_WINDOW_HIDDEN) windowFlags |= SDL_WINDOW_HIDDEN;
    if (flags & HP_FLAG_WINDOW_BORDERLESS) windowFlags |= SDL_WINDOW_BORDERLESS;
    if (flags & HP_FLAG_WINDOW_RESIZABLE) windowFlags |= SDL_WINDOW_RESIZABLE;
    if (flags & HP_FLAG_WINDOW_MINIMIZED) windowFlags |= SDL_WINDOW_MINIMIZED;
    if (flags & HP_FLAG_WINDOW_MAXIMIZED) windowFlags |= SDL_WINDOW_MAXIMIZED;
    if (flags & HP_FLAG_WINDOW_TOPMOST) windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (flags & HP_FLAG_WINDOW_TRANSPARENT) windowFlags |= SDL_WINDOW_TRANSPARENT;
    if (flags & HP_FLAG_WINDOW_NOT_FOCUSABLE) windowFlags |= SDL_WINDOW_NOT_FOCUSABLE;
    if (flags & HP_FLAG_MOUSE_GRABBED) windowFlags |= SDL_WINDOW_MOUSE_GRABBED;
    if (flags & HP_FLAG_MOUSE_CAPTURE) windowFlags |= SDL_WINDOW_MOUSE_CAPTURE;
    if (flags & HP_FLAG_MOUSE_RELATIVE) windowFlags |= SDL_WINDOW_MOUSE_RELATIVE_MODE;
    if (flags & HP_FLAG_MOUSE_FOCUS) windowFlags |= SDL_WINDOW_MOUSE_FOCUS;
    if (flags & HP_FLAG_INPUT_FOCUS) windowFlags |= SDL_WINDOW_INPUT_FOCUS;
    if (flags & HP_FLAG_KEYBOARD_GRABBED) windowFlags |= SDL_WINDOW_KEYBOARD_GRABBED;
    if (flags & HP_FLAG_HIGH_PIXEL_DENSITY) windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    return windowFlags;
}

#endif // HP_CORE_STATE_HPP
