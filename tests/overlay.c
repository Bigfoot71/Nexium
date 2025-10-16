#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_AppDesc desc = {
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
        .flags = NX_FLAG_VSYNC_HINT
    };

    NX_InitEx("Nexium - Overlay", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Texture* texture = NX_LoadTexture("images/uv-grid.png");
    NX_SetTextureFilter(texture, NX_TEXTURE_FILTER_BILINEAR);

    NX_Font* font = NX_LoadFont("fonts/Eater-Regular.ttf", NX_FONT_SDF, 32, NULL, 0);

    NX_Mesh* quad = NX_GenMeshQuad(NX_VEC2(100, 100), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_ONE, NX_VEC3_ONE);

    NX_Material matQuad = NX_GetDefaultMaterial();
    matQuad.albedo.color = NX_GREEN;

    NX_Material matCube = NX_GetDefaultMaterial();
    matCube.albedo.color = NX_BLUE;

    NX_Light* light = NX_CreateLight(NX_LIGHT_SPOT);
    NX_SetLightPosition(light, NX_VEC3(-2, 5, 2));
    NX_SetLightDirection(light, NX_VEC3(1,-1,-1));
    NX_SetLightRange(light, 16.0f);
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 5.0f, 2.5f);

        NX_Begin3D(&camera, NULL, NULL);
        {
            NX_DrawMesh3D(quad, &matQuad, &(NX_Transform) {
                NX_VEC3(0, -0.5f, 0), NX_QUAT_IDENTITY, NX_VEC3_ONE
            });
            NX_DrawMesh3D(cube, &matCube, NULL);
        }
        NX_End3D();

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_COLOR(0.25f, 0.25f, 0.25f, 0.75f));
            NX_DrawRectRounded2D(4, 4, 250, NX_GetWindowHeight() - 8, 16.0f, 8);

            NX_SetColor2D(NX_GRAY);
            NX_DrawRectRoundedBorder2D(4, 4, 250, NX_GetWindowHeight() - 8, 16.0f, 8, 2.0f);

            NX_SetFont2D(NULL);
            NX_SetColor2D(NX_WHITE);
            NX_DrawText2D(CMN_FormatText("FPS: %i", NX_GetFPS()), NX_VEC2(10, 10), 32, NX_VEC2(2, 2));

            NX_SetFont2D(font);
            NX_SetColor2D(NX_ColorFromHSV(90 * NX_GetElapsedTime(), 1.0f, 1.0f, 1.0f));
            NX_DrawText2D("Hello World!", NX_VEC2(10, 68), NX_PingPong(8 * NX_GetElapsedTime(), 24, 28), NX_VEC2(2, 2));

            NX_SetColor2D(NX_RED);
            NX_DrawPieSliceBorder2D(NX_VEC2(50, 160), 32.0f, 0.0f, NX_PingPong(NX_GetElapsedTime(), 0.0f, NX_TAU), 16, 2.0f);

            NX_SetColor2D(NX_COLOR(0.0f, 0.75f, 0.75f, 0.25f));
            NX_DrawPieSlice2D(NX_VEC2(50, 160), 32.0f, 0.0f, NX_PingPong(NX_GetElapsedTime(), 0.0f, NX_TAU), 16);

            NX_SetColor2D(NX_GREEN);
            NX_DrawArc2D(NX_VEC2(150, 160), 32.0f, 0.0f, NX_PingPong(NX_GetElapsedTime(), 0.0f, NX_TAU), 16, 2.0f);

            NX_SetColor2D(NX_YELLOW);
            NX_DrawBezierCubic2D(NX_VEC2(50, 240), NX_VEC2(100, 210), NX_VEC2(150, 270), NX_VEC2(200, 240), 16, 2.0f);

            NX_SetTexture2D(texture);
            NX_SetColor2D(NX_WHITE);
            {
                if ((int)(NX_GetElapsedTime()) % 2 == 0) {
                    NX_DrawRect2D(63, 286, 128, 128);
                }
                else {
                    NX_DrawCircle2D(NX_VEC2(127, 350), 64, 32);
                }
            }

            NX_SetTexture2D(NULL);
        }
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
