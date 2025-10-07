#ifndef COMMON_H
#define COMMON_H

#include <NX/Nexium.h>
#include <stdio.h>

static inline const char* CMN_FormatText(const char* fmt, ...)
{
    static char buffer[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    return buffer;
}

static inline void CMN_UpdateCamera(NX_Camera* camera, NX_Vec3 center, float distance, float height)
{
    static bool mouseCaptured = false;

    if (!mouseCaptured) {
        NX_UpdateCameraOrbital(camera, center, distance, height, NX_GetElapsedTime());

        if (NX_IsMouseButtonJustPressed(NX_MOUSE_BUTTON_LEFT | NX_MOUSE_BUTTON_RIGHT)) {
            NX_CaptureMouse(true);
            mouseCaptured = true;
        }
    }
    else {
        NX_Vec3 move = NX_GetKeyVec3(NX_KEY_W, NX_KEY_S, NX_KEY_A, NX_KEY_D);
        move = NX_Vec3Scale(move, 10.0f * NX_GetFrameTime());

        NX_Vec2 rot = NX_Vec2Scale(NX_GetMouseDelta(), -0.01f);

        NX_UpdateCameraFree(camera, move, NX_VEC3(rot.y, rot.x, 0.0f), -80.0f);

        if (NX_IsMouseButtonJustPressed(NX_MOUSE_BUTTON_LEFT | NX_MOUSE_BUTTON_RIGHT)) {
            NX_CaptureMouse(false);
            mouseCaptured = false;
        }
    }
}

#endif // COMMON_H
