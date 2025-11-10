/* render_texture.c -- Test rendering to a render texture and blitting it on screen
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"
#include "NX/NX_Render3D.h"

int main(void)
{
    /* --- Initialize engine --- */

    NX_Init("Nexium - Render Texture", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    /* --- Create render target --- */

    NX_RenderTexture* target = NX_CreateRenderTexture(1920, 1080);

    /* --- Load 3D resources --- */

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        /* Update camera */

        CMN_UpdateCamera(&camera, NX_VEC3(0, 1, 0), 2.0f, 1.0f);

        /* --- Render 3D scene to target --- */

        NX_BeginShadow3D(light, &camera, 0);
        {
            NX_DrawMesh3D(ground, NULL, NULL);
            NX_DrawModel3D(model, NULL);
        }
        NX_EndShadow3D();

        NX_BeginEx3D(&camera, NULL, target, 0);
        {
            NX_DrawMesh3D(ground, NULL, NULL);
            NX_DrawModel3D(model, NULL);
        }
        NX_End3D();

        /* --- Draw 2D overlay on target --- */

        NX_Begin2D(target);
        NX_SetColor2D(NX_YELLOW);
        NX_DrawText2D("Hello, I'm blit on the screen!", NX_VEC2(10, 10), 128, NX_VEC2_ONE);
        NX_End2D();

        /* --- Blit render texture to screen --- */

        NX_BlitRenderTexture(target, 50, 50, 700, 350, true);
    }

    /* --- Cleanup --- */

    NX_DestroyRenderTexture(target);
    NX_DestroyMesh(ground);
    NX_DestroyModel(model);
    NX_DestroyLight(light);

    NX_Quit();

    return 0;
}
