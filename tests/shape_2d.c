#include <NX/Nexium.h>
#include "./common.h"
#include "NX/NX_Core.h"
#include "NX/NX_Init.h"
#include "NX/NX_Render.h"

/* === Points Test Arrays === */

static const NX_Vec2 points[] = {
    {-0.5f, -0.5f},
    {+0.5f, -0.5f},
    {-0.5f, +0.5f},
    {+0.5f, +0.5f}
};

static const NX_Vec2 lines[] = {
    {-0.5f, -0.5f},
    {+0.5f, -0.5f},
    {-0.5f, +0.5f},
    {+0.5f, +0.5f}
};

static const NX_Vec2 lineStrip[] = {
    {-0.5f, -0.5f},
    {+0.5f, -0.5f},
    {+0.5f, +0.5f},
    {-0.5f, +0.5f}
};

static const NX_Vec2 lineLoop[] = {
    {-0.5f, -0.5f},
    {+0.5f, -0.5f},
    {0.5f, +0.5f}
};

static const NX_Vec2 triangles[] = {
    {-0.5f, -0.5f},
    {+0.5f, -0.5f},
    {-0.5f, +0.5f},
    {+0.5f, -0.5f},
    {+0.5f, +0.5f},
    {-0.5f, +0.5f}
};

static const NX_Vec2 triangleStrip[] = {
    {-0.5f, -0.5f},
    {+0.5f, -0.5f},
    {-0.5f, +0.5f},
    {+0.5f, +0.5f}
};

static const NX_Vec2 triangleFan[] = {
    {-0.5f, -0.5f},
    {+0.5f, -0.5f},
    {+0.5f, +0.5f},
    {-0.5f, +0.5f}
};

/* === Primitive Names === */

static const char* primitives[] = {
    [NX_PRIMITIVE_POINTS]         = "Points",
    [NX_PRIMITIVE_LINES]          = "Lines",
    [NX_PRIMITIVE_LINE_STRIP]     = "Line Strip",
    [NX_PRIMITIVE_LINE_LOOP]      = "Line Loop",
    [NX_PRIMITIVE_TRIANGLES]      = "Triangles",
    [NX_PRIMITIVE_TRIANGLE_STRIP] = "Triangle Strip",
    [NX_PRIMITIVE_TRIANGLE_FAN]   = "Triangle Fan"
};

/* === Program === */

int main(void)
{
    NX_AppDesc desc = {
        .render2D.resolution.x = 800,
        .render2D.resolution.y = 450,
        .flags = NX_FLAG_VSYNC_HINT,
        .targetFPS = 60
    };

    NX_InitEx("Nexium - Shape 2D", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_PrimitiveType currentPrimitive = NX_PRIMITIVE_POINTS;
    float thickness = 0.0f;

    while (NX_FrameStep())
    {
        if (NX_IsKeyJustPressed(NX_KEY_SPACE)) {
            currentPrimitive = (currentPrimitive + 1) % NX_ARRAY_SIZE(primitives);
        }

        thickness += NX_GetMouseWheel().y * 0.5f;

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_BLACK);
            NX_DrawRect2D(0, 0, NX_GetWindowWidth(), NX_GetWindowHeight());

            NX_Push2D();
            NX_Scale2D(NX_VEC2_1(100));
            NX_Translate2D(NX_Vec2Scale(NX_GetWindowSizeF(), 0.5f));

            NX_SetColor2D(NX_RED);
            switch (currentPrimitive) {
            case NX_PRIMITIVE_POINTS:
                NX_DrawShape2D(currentPrimitive, points, NX_ARRAY_SIZE(points), thickness);
                break;
            case NX_PRIMITIVE_LINES:
                NX_DrawShape2D(currentPrimitive, lines, NX_ARRAY_SIZE(lines), thickness);
                break;
            case NX_PRIMITIVE_LINE_STRIP:
                NX_DrawShape2D(currentPrimitive, lineStrip, NX_ARRAY_SIZE(lineStrip), thickness);
                break;
            case NX_PRIMITIVE_LINE_LOOP:
                NX_DrawShape2D(currentPrimitive, lineLoop, NX_ARRAY_SIZE(lineLoop), thickness);
                break;
            case NX_PRIMITIVE_TRIANGLES:
                NX_DrawShape2D(currentPrimitive, triangles, NX_ARRAY_SIZE(triangles), thickness);
                break;
            case NX_PRIMITIVE_TRIANGLE_STRIP:
                NX_DrawShape2D(currentPrimitive, triangleStrip, NX_ARRAY_SIZE(triangleStrip), thickness);
                break;
            case NX_PRIMITIVE_TRIANGLE_FAN:
                NX_DrawShape2D(currentPrimitive, triangleFan, NX_ARRAY_SIZE(triangleFan), thickness);
                break;
            }

            NX_Pop2D();

            NX_SetColor2D(NX_YELLOW);
            NX_DrawText2D(
                CMN_FormatText("Primitive: %s", primitives[currentPrimitive]),
                NX_VEC2(10, 10), 16, NX_VEC2_ONE
            );
            NX_DrawText2D(
                CMN_FormatText("Thickness: %f", thickness),
                NX_VEC2(10, NX_GetWindowHeight() - 26),
                16, NX_VEC2_ONE
            );
        }
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
