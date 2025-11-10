/* lights.c -- Stress test for multiple lights and individually rendered meshes
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
    /* --- Initialize application --- */

    NX_AppDesc desc = {
        .render3D.resolution.x = 800,
        .render3D.resolution.y = 450,
        .render3D.sampleCount  = 4,
        .targetFPS             = 60
    };

    NX_InitEx("Nexium - Lights", 800, 450, &desc);

    /* --- Generate meshes --- */

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(100.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube   = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_IVEC3_ONE);
    NX_Mesh* sphere = NX_GenMeshSphere(0.1f, 16, 8);

    /* --- Setup unlit material for light markers --- */

    NX_Material matSphere = NX_GetDefaultMaterial();
    matSphere.shading = NX_SHADING_UNLIT;

    /* --- Create random omni lights --- */

    NX_Light* lights[256];

    for (int i = 0; i < NX_ARRAY_SIZE(lights); i++) {
        lights[i] = NX_CreateLight(NX_LIGHT_OMNI);

        NX_SetLightPosition(lights[i], NX_VEC3(
            NX_RandRangeFloat(NULL, -50.0f, 50.0f),
            NX_RandRangeFloat(NULL,  2.0f,  4.0f),
            NX_RandRangeFloat(NULL, -50.0f, 50.0f)
        ));

        NX_SetLightColor(lights[i], NX_ColorFromHSV(
            360.0f * NX_RandFloat(NULL), 1.0f, 1.0f, 1.0f
        ));

        NX_SetLightActive(lights[i], true);
    }

    /* --- Setup camera --- */

    NX_Camera camera = NX_GetDefaultCamera();

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 16.0f, 8.0f);

        /* --- 3D rendering --- */

        NX_Begin3D(&camera, NULL, 0);
        {
            NX_DrawMesh3D(ground, NULL, NULL);

            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            /* --- Draw large field of cubes --- */

            for (int z = -45; z <= 45; z++) {
                for (int x = -45; x <= 45; x++) {
                    transform.translation = NX_VEC3(x, 0.25f, z);
                    NX_DrawMesh3D(cube, NULL, &transform);
                }
            }

            /* --- Draw spheres at light positions --- */

            for (int i = 0; i < NX_ARRAY_SIZE(lights); i++) {
                transform.translation   = NX_GetLightPosition(lights[i]);
                matSphere.albedo.color  = NX_GetLightColor(lights[i]);
                NX_DrawMesh3D(sphere, &matSphere, &transform);
            }
        }
        NX_End3D();

        /* --- 2D overlay --- */

        NX_Begin2D(NULL);
        NX_SetColor2D(NX_BLACK);
        NX_DrawText2D(
            CMN_FormatText("FPS: %i", NX_GetFPS()),
            NX_VEC2(10, 10), 16, NX_VEC2_ONE
        );
        NX_End2D();
    }

    /* --- Cleanup --- */

    NX_DestroyMesh(ground);
    NX_DestroyMesh(cube);
    NX_DestroyMesh(sphere);

    for (int i = 0; i < NX_ARRAY_SIZE(lights); i++) {
        NX_DestroyLight(lights[i]);
    }

    NX_Quit();

    return 0;
}
