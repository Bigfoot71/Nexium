/* shading_mode.c -- Test switching between lit and unlit shading modes
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    /* --- Initialize engine --- */

    NX_AppDesc desc = {
        .render2D = { .sampleCount = 4, .resolution = {800, 450} },
        .render3D = { .sampleCount = 4, .resolution = {800, 450} },
        .flags = NX_FLAG_VSYNC_HINT
    };

    NX_InitEx("Nexium - Shading Mode", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    /* --- Create meshes and material --- */

    NX_Mesh* quad = NX_GenMeshQuad(NX_VEC2(100, 100), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_ONE, NX_IVEC3_ONE);
    NX_Material material = NX_GetDefaultMaterial();

    /* --- Setup light --- */

    NX_Light* light = NX_CreateLight(NX_LIGHT_SPOT);
    NX_SetLightPosition(light, NX_VEC3(-2, 5, 2));
    NX_SetLightDirection(light, NX_VEC3(1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        /* Update camera */
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 5.0f, 2.5f);

        /* Switch shading mode with SPACE */
        if (NX_IsKeyJustPressed(NX_KEY_SPACE))
            material.shading = (material.shading + 1) % 2;

        /* --- 3D rendering --- */

        NX_Begin3D(&camera, NULL, NULL);

        /* Draw ground quad */
        material.albedo.color = NX_GREEN;
        NX_DrawMesh3D(quad, &material, &(NX_Transform){ NX_VEC3(0, -0.501f, 0), NX_QUAT_IDENTITY, NX_VEC3_ONE });

        /* Draw cube */
        material.albedo.color = NX_BLUE;
        NX_DrawMesh3D(cube, &material, NULL);

        NX_End3D();

        /* --- 2D UI --- */
    
        NX_Begin2D(NULL);
        NX_SetColor2D(NX_BLACK);
        NX_DrawText2D("Press SPACE to change the shading mode", NX_VEC2_1(10), 24, NX_VEC2_ONE);
        NX_End2D();
    }

    /* --- Cleanup --- */

    NX_DestroyMesh(quad);
    NX_DestroyMesh(cube);
    NX_DestroyLight(light);

    NX_Quit();

    return 0;
}
