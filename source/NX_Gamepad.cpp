#include "SDL3/SDL_gamepad.h"

#include "INX_GlobalState.hpp"
#include <NX/NX_Gamepad.h>
#include <algorithm>

// ============================================================
// PRIVATE HELPERS
// ============================================================

static INX_GamepadState::Device* INX_GetDevice(int index)
{
    if (index < 0 || index >= INX_GamepadState::MaxGampads) {
        return nullptr;
    }

    INX_GamepadState::Device& dev = INX_Gamepad.devices[index];

    return (dev.gamepad != nullptr) ? &dev : nullptr;
}

static SDL_Gamepad* INX_GetGamepad(int index)
{
    if (index < 0 || index >= INX_GamepadState::MaxGampads) {
        return nullptr;
    }

    return INX_Gamepad.devices[index].gamepad;
}

static NX_Vec2 INX_SquareToCircle(NX_Vec2 v)
{
    NX_Vec2 o;
    o.x = v.x * sqrtf(1.0f - (v.y * v.y) / 2.0f);
    o.y = v.y * sqrtf(1.0f - (v.x * v.x) / 2.0f);
    return o;
}

// ============================================================
// PUBLIC API
// ============================================================

bool NX_IsGamepadAvailable(int gamepad)
{
    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    return (gp != nullptr);
}

NX_GamepadType NX_GetGamepadType(int gamepad)
{
    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    if (gp == nullptr) return NX_GAMEPAD_TYPE_UNKNOWN;

    return static_cast<NX_GamepadType>(SDL_GetGamepadType(gp));
}

const char* NX_GetGamepadName(int gamepad)
{
    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    if (gp == nullptr) return nullptr;

    return SDL_GetGamepadName(gp);
}

bool NX_IsGamepadButtonPressed(int gamepad, NX_GamepadButton button)
{
    INX_GamepadState::Device* dev = INX_GetDevice(gamepad);
    if (dev == nullptr) return false;

    return (dev->buttons[button] & 0xF0);
}

bool NX_IsGamepadButtonReleased(int gamepad, NX_GamepadButton button)
{
    INX_GamepadState::Device* dev = INX_GetDevice(gamepad);
    if (dev == nullptr) return false;

    return !(dev->buttons[button] & 0xF0);
}

bool NX_IsGamepadButtonJustPressed(int gamepad, NX_GamepadButton button)
{
    INX_GamepadState::Device* dev = INX_GetDevice(gamepad);
    if (dev == nullptr) return false;

    bool current = (dev->buttons[button] & 0xF0);
    bool previous = (dev->buttons[button] & 0x0F);

    return (current && !previous);
}

bool NX_IsGamepadButtonJustReleased(int gamepad, NX_GamepadButton button)
{
    INX_GamepadState::Device* dev = INX_GetDevice(gamepad);
    if (dev == nullptr) return false;

    bool current = (dev->buttons[button] & 0xF0);
    bool previous = (dev->buttons[button] & 0x0F);

    return (!current && previous);
}

bool NX_HasGamepadAxis(int gamepad, NX_GamepadAxis axis)
{
    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    if (gp == nullptr) return false;

    return SDL_GamepadHasAxis(gp, static_cast<SDL_GamepadAxis>(axis));
}

float NX_GetGamepadAxis(int gamepad, NX_GamepadAxis axis)
{
    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    if (gp == nullptr) return 0.0f;

    Sint16 v = SDL_GetGamepadAxis(gp, static_cast<SDL_GamepadAxis>(axis));
    return static_cast<float>(v) / 32767.0f;
}

NX_Vec2 NX_GetGamepadLeftStick(int gamepad)
{
    NX_Vec2 v = NX_VEC2_ZERO;

    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    if (gp == nullptr) return v;

    Sint16 x = SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_LEFTX);
    Sint16 y = SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_LEFTY);

    v.x = static_cast<float>(x) / 32767.0f;
    v.y = static_cast<float>(y) / 32767.0f;

    return INX_SquareToCircle(v);
}

NX_Vec2 NX_GetGamepadRightStick(int gamepad)
{
    NX_Vec2 v = NX_VEC2_ZERO;

    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    if (gp == nullptr) return v;

    Sint16 x = SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_RIGHTX);
    Sint16 y = SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_RIGHTY);

    v.x = static_cast<float>(x) / 32767.0f;
    v.y = static_cast<float>(y) / 32767.0f;

    return INX_SquareToCircle(v);
}

void NX_RumbleGamepad(int gamepad, float left, float right, float duration)
{
    if (duration <= 0.0f) return;

    SDL_Gamepad* gp = INX_GetGamepad(gamepad);
    if (!gp) return;

    Uint32 d = static_cast<Uint32>(duration * 1000.0f);
    Uint16 l = static_cast<Uint16>(std::clamp(left, 0.0f, 1.0f) * 65535.0f);
    Uint16 r = static_cast<Uint16>(std::clamp(right, 0.0f, 1.0f) * 65535.0f);

    SDL_RumbleGamepad(gp, l, r, d);
}
