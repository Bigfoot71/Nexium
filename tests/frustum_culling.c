/* frustum_culling.c -- Frustum culling test useful when measuring performance
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
    NX_AppDesc desc = {
        .render3D.resolution.x = 800,
        .render3D.resolution.y = 600,
        .render3D.sampleCount = 4,
        //.targetFPS = 60
    };

    NX_InitEx("Nexium - Frustum Culling", 800, 450, &desc);

    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_IVEC3_ONE);
    NX_Material material = NX_GetDefaultMaterial();
    material.shading = NX_SHADING_UNLIT;

    while (NX_FrameStep())
    {
        NX_Begin3D(NULL, NULL, NULL);
        {
            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (int z = -10; z <= 10; z++) {
                for (int y = -10; y <= 10; y++) {
                    for (float x = -10; x <= 10; x++) {
                        transform.translation = NX_VEC3(x, y, z);
                        material.albedo.color = NX_ColorFromHSV(NX_Remap(x, -10, 10, 0, 360), 1, 1, 1);
                        NX_DrawMesh3D(cube, &material, &transform);
                    }
                }
            }
        }
        NX_End3D();

        NX_Begin2D(NULL);
        NX_SetColor2D(NX_BLACK);
        NX_DrawText2D(CMN_FormatText("FPS: %i", NX_GetFPS()), NX_VEC2(10, 10), 16, NX_VEC2_ONE);
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
