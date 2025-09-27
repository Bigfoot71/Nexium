#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_AppDesc desc = {
        .render2D = {
            .sampleCount = 4,
            .resolution.x = 800,
            .resolution.y = 450
        },
        .render3D = {
            .sampleCount = 4,
            .resolution.x = 800,
            .resolution.y = 450
        },
        .flags = HP_FLAG_VSYNC_HINT
    };

    HP_InitEx("Hyperion - Overlay", 800, 450, &desc);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_Texture* texture = HP_LoadTexture("images/uv-grid.png");
    HP_SetTextureFilter(texture, HP_TEXTURE_FILTER_BILINEAR);

    HP_Font* font = HP_LoadFont("fonts/Eater-Regular.ttf", HP_FONT_SDF, 32, NULL, 0);

    HP_Mesh* quad = HP_GenMeshQuad(HP_VEC2(100, 100), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_ONE, HP_VEC3_ONE);

    HP_Material matQuad = HP_GetDefaultMaterial();
    matQuad.albedo.color = HP_GREEN;

    HP_Material matCube = HP_GetDefaultMaterial();
    matCube.albedo.color = HP_BLUE;

    HP_Light* light = HP_CreateLight(HP_LIGHT_SPOT);
    HP_SetLightPosition(light, HP_VEC3(-2, 5, 2));
    HP_SetLightDirection(light, HP_VEC3(1,-1,-1));
    HP_SetShadowActive(light, true);
    HP_SetLightActive(light, true);

    HP_Camera camera = HP_GetDefaultCamera();

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3_ZERO, 5.0f, 2.5f);

        HP_Begin3D(&camera, NULL, NULL);
        {
            HP_DrawMesh3D(quad, &matQuad, &(HP_Transform) {
                HP_VEC3(0, -0.5f, 0), HP_QUAT_IDENTITY, HP_VEC3_ONE
            });
            HP_DrawMesh3D(cube, &matCube, NULL);
        }
        HP_End3D();

        HP_Begin2D(NULL);
        {
            HP_SetColor2D(HP_COLOR(0.25f, 0.25f, 0.25f, 0.75f));
            HP_DrawRectRounded2D(4, 4, 250, HP_GetWindowHeight() - 8, 16.0f, 8);

            HP_SetColor2D(HP_GRAY);
            HP_DrawRectRoundedBorder2D(4, 4, 250, HP_GetWindowHeight() - 8, 16.0f, 8, 2.0f);

            HP_SetFont2D(NULL);
            HP_SetColor2D(HP_WHITE);
            HP_DrawText2D(CMN_FormatText("FPS: %i", HP_GetFPS()), HP_VEC2(10, 10), 32, HP_VEC2(2, 2));

            HP_SetFont2D(font);
            HP_SetColor2D(HP_ColorFromHSV(90 * HP_GetElapsedTime(), 1.0f, 1.0f, 1.0f));
            HP_DrawText2D("Hello World!", HP_VEC2(10, 68), HP_PingPong(8 * HP_GetElapsedTime(), 24, 28), HP_VEC2(2, 2));

            HP_SetColor2D(HP_RED);
            HP_DrawPieSliceBorder2D(HP_VEC2(50, 160), 32.0f, 0.0f, HP_PingPong(HP_GetElapsedTime(), 0.0f, HP_TAU), 16, 2.0f);

            HP_SetColor2D(HP_COLOR(0.0f, 0.75f, 0.75f, 0.25f));
            HP_DrawPieSlice2D(HP_VEC2(50, 160), 32.0f, 0.0f, HP_PingPong(HP_GetElapsedTime(), 0.0f, HP_TAU), 16);

            HP_SetColor2D(HP_GREEN);
            HP_DrawArc2D(HP_VEC2(150, 160), 32.0f, 0.0f, HP_PingPong(HP_GetElapsedTime(), 0.0f, HP_TAU), 16, 2.0f);

            HP_SetColor2D(HP_YELLOW);
            HP_DrawBezierCubic2D(HP_VEC2(50, 240), HP_VEC2(100, 210), HP_VEC2(150, 270), HP_VEC2(200, 240), 16, 2.0f);

            HP_SetTexture2D(texture);
            HP_SetColor2D(HP_WHITE);
            {
                if ((int)(HP_GetElapsedTime() / 3.0f) % 2 == 0) {
                    HP_DrawRectEx2D(HP_VEC2(127, 350), HP_VEC2_1(128), HP_VEC2(0.5f, 0.5f), HP_TAU / 3.0f * HP_GetElapsedTime());
                }
                else {
                    HP_DrawCircle2D(HP_VEC2(127, 350), 64, 32);
                }
            }

            HP_SetTexture2D(NULL);
        }
        HP_End2D();
    }

    HP_Quit();

    return 0;
}
