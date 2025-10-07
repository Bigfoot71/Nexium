/* NX_CoreState.hpp -- Contains and manages the global core state of Nexium
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CORE_STATE_HPP
#define NX_CORE_STATE_HPP

#include <NX/NX_Init.h>
#include <NX/NX_Core.h>
#include <NX/NX_Math.h>

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

#include <memory>

/* === Glabal State === */

extern std::unique_ptr<class NX_CoreState> gCore;

/* === Declaration === */

class NX_CoreState {
private:
    friend bool ::NX_InitEx(const char* title, int w, int h, NX_AppDesc* desc);
    friend void ::NX_SetMousePosition(NX_Vec2 p);
    friend bool ::NX_FrameStep(void);

public:
    /** Core data */
    SDL_Window* window() const;

    /** Framerate and time */
    void setTargetFrameRate(int fps) const;
    double elapsedTime() const;
    double frameTime() const;
    double frameRate() const;

    /** Input control */
    bool currentKey(NX_Key key) const;
    bool previousKey(NX_Key key) const;
    bool currentMouseButton(NX_MouseButton buttons);
    bool previousMouseButton(NX_MouseButton buttons);
    const NX_Vec2& mousePosition() const;
    const NX_Vec2& mouseDelta() const;
    const NX_Vec2& mouseWheel() const;

    /** OpenGL infos */
    SDL_GLProfile glProfile() const;

public:
    NX_CoreState(const char* title, int w, int h, const NX_AppDesc& desc);
    ~NX_CoreState();

private:
    static SDL_WindowFlags sdlWindowFlags(NX_Flags flags);

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
    NX_Vec2 mMousePosition{};
    NX_Vec2 mMouseDelta{};
    NX_Vec2 mMouseWheel{};

    /** OpenGL infos */
    SDL_GLProfile mGLProfile{};
};

/* === Public Implementation === */

inline SDL_Window* NX_CoreState::window() const
{
    return mWindow;
}

inline void NX_CoreState::setTargetFrameRate(int fps) const
{
    gCore->mTargetFrameTime = 1.0 / static_cast<double>(fps);
}

inline double NX_CoreState::elapsedTime() const
{
    return mElapsedTime;
}

inline double NX_CoreState::frameTime() const
{
    return mCurrentFrameTime;
}

inline double NX_CoreState::frameRate() const
{
    return mFpsAverage;
}

inline SDL_GLProfile NX_CoreState::glProfile() const
{
    return mGLProfile;
}

inline bool NX_CoreState::currentKey(NX_Key key) const
{
    SDL_assert(static_cast<uint32_t>(key) < SDL_SCANCODE_COUNT);

    return mKeys[key] & 0xF0;
}

inline bool NX_CoreState::previousKey(NX_Key key) const
{
    SDL_assert(static_cast<uint32_t>(key) < SDL_SCANCODE_COUNT);

    return mKeys[key] & 0x0F;
}

inline bool NX_CoreState::currentMouseButton(NX_MouseButton buttons)
{
    return (gCore->mMouseButtons[0] & buttons);
}

inline bool NX_CoreState::previousMouseButton(NX_MouseButton buttons)
{
    return (gCore->mMouseButtons[1] & buttons);
}

inline const NX_Vec2& NX_CoreState::mousePosition() const
{
    return mMousePosition;
}

inline const NX_Vec2& NX_CoreState::mouseDelta() const
{
    return mMouseDelta;
}

inline const NX_Vec2& NX_CoreState::mouseWheel() const
{
    return mMouseWheel;
}

/* === Private Implementation === */

inline SDL_WindowFlags NX_CoreState::sdlWindowFlags(NX_Flags flags)
{
    SDL_WindowFlags windowFlags = 0;
    if (flags & NX_FLAG_FULLSCREEN) windowFlags |= SDL_WINDOW_FULLSCREEN;
    if (flags & NX_FLAG_WINDOW_OCCLUDED) windowFlags |= SDL_WINDOW_OCCLUDED;
    if (flags & NX_FLAG_WINDOW_HIDDEN) windowFlags |= SDL_WINDOW_HIDDEN;
    if (flags & NX_FLAG_WINDOW_BORDERLESS) windowFlags |= SDL_WINDOW_BORDERLESS;
    if (flags & NX_FLAG_WINDOW_RESIZABLE) windowFlags |= SDL_WINDOW_RESIZABLE;
    if (flags & NX_FLAG_WINDOW_MINIMIZED) windowFlags |= SDL_WINDOW_MINIMIZED;
    if (flags & NX_FLAG_WINDOW_MAXIMIZED) windowFlags |= SDL_WINDOW_MAXIMIZED;
    if (flags & NX_FLAG_WINDOW_TOPMOST) windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (flags & NX_FLAG_WINDOW_TRANSPARENT) windowFlags |= SDL_WINDOW_TRANSPARENT;
    if (flags & NX_FLAG_WINDOW_NOT_FOCUSABLE) windowFlags |= SDL_WINDOW_NOT_FOCUSABLE;
    if (flags & NX_FLAG_MOUSE_GRABBED) windowFlags |= SDL_WINDOW_MOUSE_GRABBED;
    if (flags & NX_FLAG_MOUSE_CAPTURE) windowFlags |= SDL_WINDOW_MOUSE_CAPTURE;
    if (flags & NX_FLAG_MOUSE_RELATIVE) windowFlags |= SDL_WINDOW_MOUSE_RELATIVE_MODE;
    if (flags & NX_FLAG_MOUSE_FOCUS) windowFlags |= SDL_WINDOW_MOUSE_FOCUS;
    if (flags & NX_FLAG_INPUT_FOCUS) windowFlags |= SDL_WINDOW_INPUT_FOCUS;
    if (flags & NX_FLAG_KEYBOARD_GRABBED) windowFlags |= SDL_WINDOW_KEYBOARD_GRABBED;
    if (flags & NX_FLAG_HIGH_PIXEL_DENSITY) windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    return windowFlags;
}

#endif // NX_CORE_STATE_HPP
