#ifndef COMMON_H
#define COMMON_H

#include <Hyperion/Hyperion.h>
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

static inline void CMN_UpdateCamera(HP_Camera* camera, HP_Vec3 center, float distance, float height)
{
    static bool mouseCaptured = false;

    if (!mouseCaptured) {
        HP_UpdateCameraOrbital(camera, center, distance, height, HP_GetElapsedTime());

        if (HP_IsMouseButtonJustPressed(HP_MOUSE_BUTTON_LEFT | HP_MOUSE_BUTTON_RIGHT)) {
            HP_CaptureMouse(true);
            mouseCaptured = true;
        }
    }
    else {
        HP_Vec3 move = HP_GetKeyVec3(HP_KEY_W, HP_KEY_S, HP_KEY_A, HP_KEY_D);
        move = HP_Vec3Scale(move, 10.0f * HP_GetFrameTime());

        HP_Vec2 rot = HP_Vec2Scale(HP_GetMouseDelta(), -0.01f);

        HP_UpdateCameraFree(camera, move, HP_VEC3(rot.y, rot.x, 0.0f), -80.0f);

        if (HP_IsMouseButtonJustPressed(HP_MOUSE_BUTTON_LEFT | HP_MOUSE_BUTTON_RIGHT)) {
            HP_CaptureMouse(false);
            mouseCaptured = false;
        }
    }
}

#endif // COMMON_H
