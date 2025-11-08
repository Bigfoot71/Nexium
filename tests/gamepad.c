#include "NX/NX_Gamepad.h"
#include "NX/NX_Render2D.h"
#include <NX/Nexium.h>

static void DrawStick(NX_Vec2 position, NX_Vec2 stick, bool pressed)
{
    float radius = pressed ? 48 : 64;

    NX_SetColor2D(NX_GRAY);
    NX_DrawCircleBorder2D(position, radius, 16, 2.0f);

    stick = NX_Vec2Add(position, NX_Vec2Scale(stick, radius));
    NX_DrawCircle2D(stick, 8, 8);
}

static void DrawButton(NX_Vec2 position, char letter, bool pressed, NX_Color color)
{
    float radius = pressed ? 24 : 32;

    NX_SetColor2D(NX_ColorScale(color, 0.5f));
    NX_DrawCircle2D(position, radius, 16);

    NX_Vec2 letterPos = NX_Vec2Sub(position, NX_VEC2(radius * 0.25f, radius * 0.5f));

    NX_SetColor2D(color);
    NX_DrawCircleBorder2D(position, radius, 16, 3);
    NX_DrawCodepoint2D(letter, letterPos, radius);
}

static void DrawArrow(NX_Vec2 position, float rot, bool pressed)
{
    float radius = pressed ? 24.0f : 32.0f;

    NX_Vec2 arrow[7] = {
        NX_VEC2(  0.0f,  1.0f),
        NX_VEC2(  0.5f,  0.0f),
        NX_VEC2(  0.2f,  0.0f),
        NX_VEC2(  0.2f, -1.0f),
        NX_VEC2( -0.2f, -1.0f),
        NX_VEC2( -0.2f,  0.0f),
        NX_VEC2( -0.5f,  0.0f),
    };

    NX_Push2D();

    NX_Scale2D(NX_VEC2_1(radius));
    NX_Rotate2D(-rot);
    NX_Translate2D(position);

    NX_SetColor2D(NX_GRAY);
    NX_DrawShape2D(NX_PRIMITIVE_TRIANGLE_FAN, arrow, 7, 0.0f);

    NX_Pop2D();
}

static void DrawTrigger(NX_Vec2 position, float pressure)
{
    float thickness = 2.0f;

    NX_SetColor2D(NX_WHITE);
    NX_DrawLine2D(
        NX_VEC2(position.x - 32, position.y - 64 + 128 * pressure),
        NX_VEC2(position.x + 32, position.y - 64 + 128 * pressure),
        thickness
    );

    NX_SetColor2D(NX_GRAY);
    NX_DrawRectBorder2D(
        position.x - 32,
        position.y - 64 - thickness, 64,
        128 + thickness,
        thickness
    );
}

int main(void)
{
    NX_Init("Nexium - Shape 2D", 800, 400, NX_FLAG_VSYNC_HINT);

    while (NX_FrameStep())
    {
        if (NX_IsGamepadButtonJustPressed(0, NX_GAMEPAD_BUTTON_LEFT_SHOULDER)) {
            NX_RumbleGamepad(0, 0.5f, 0.0f, 0.1f);
        }

        if (NX_IsGamepadButtonJustPressed(0, NX_GAMEPAD_BUTTON_RIGHT_SHOULDER)) {
            NX_RumbleGamepad(0, 0.0f, 0.5f, 0.1f);
        }

        NX_Vec2 lStick = NX_GetGamepadLeftStick(0);
        NX_Vec2 rStick = NX_GetGamepadRightStick(0);

        bool lStickPressed = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_LEFT_STICK);
        bool rStickPressed = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_RIGHT_STICK);

        bool aButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_SOUTH);
        bool bButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_EAST);
        bool yButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_NORTH);
        bool xButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_WEST);

        bool uButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_DPAD_UP);
        bool dButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_DPAD_DOWN);
        bool lButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_DPAD_LEFT);
        bool rButton = NX_IsGamepadButtonPressed(0, NX_GAMEPAD_BUTTON_DPAD_RIGHT);

        float lTrigger = NX_GetGamepadAxis(0, NX_GAMEPAD_AXIS_LEFT_TRIGGER);
        float rTrigger = NX_GetGamepadAxis(0, NX_GAMEPAD_AXIS_RIGHT_TRIGGER);

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_BLACK);
            NX_DrawRect2D(0, 0, 800, 450);

            DrawStick(NX_VEC2(100, 300), lStick, lStickPressed);
            DrawStick(NX_VEC2(700, 300), rStick, rStickPressed);

            DrawButton(NX_Vec2Add(NX_VEC2(650, 125), NX_VEC2(0, +64)), 'A', aButton, NX_GREEN);
            DrawButton(NX_Vec2Add(NX_VEC2(650, 125), NX_VEC2(+64, 0)), 'B', bButton, NX_RED);
            DrawButton(NX_Vec2Add(NX_VEC2(650, 125), NX_VEC2(0, -64)), 'Y', yButton, NX_ORANGE);
            DrawButton(NX_Vec2Add(NX_VEC2(650, 125), NX_VEC2(-64, 0)), 'X', xButton, NX_BLUE);

            DrawArrow(NX_Vec2Add(NX_VEC2(150, 125), NX_VEC2(0, +64)), 0, dButton);
            DrawArrow(NX_Vec2Add(NX_VEC2(150, 125), NX_VEC2(+64, 0)), NX_Radians(90), rButton);
            DrawArrow(NX_Vec2Add(NX_VEC2(150, 125), NX_VEC2(0, -64)), NX_Radians(180), uButton);
            DrawArrow(NX_Vec2Add(NX_VEC2(150, 125), NX_VEC2(-64, 0)), NX_Radians(270), lButton);

            DrawTrigger(NX_VEC2(300, 300), lTrigger);
            DrawTrigger(NX_VEC2(500, 300), rTrigger);
        }
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
