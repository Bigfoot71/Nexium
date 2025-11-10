/* shadows.c -- Test shadows for directional, spot, and omni lights
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"
#include "NX/NX_Light.h"
#include "NX/NX_Math.h"
#include "NX/NX_Render3D.h"

void DrawScene(const NX_Mesh* ground, const NX_Mesh* cube)
{
    NX_DrawMesh3D(ground, NULL, NULL);

    NX_Transform transform = NX_TRANSFORM_IDENTITY;
    for (float z = -4.0f; z <= 4.0f; z += 2.0f) {
        for (float x = -4.0f; x <= 4.0f; x += 2.0f) {
            transform.translation = NX_VEC3(x, 0.25f, z);
            NX_DrawMesh3D(cube, NULL, &transform);
        }
    }
}

int main(void)
{
    /* --- Initialize engine --- */

    NX_Init("Nexium - Shadows", 800, 450, NX_FLAG_VSYNC_HINT);

    /* --- Create meshes --- */

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube   = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_IVEC3_ONE);

    /* --- Create lights --- */

    NX_Light* lights[NX_LIGHT_TYPE_COUNT];
    for (int i = 0; i < NX_LIGHT_TYPE_COUNT; i++) {
        lights[i] = NX_CreateLight(i);
    }

    /* --- Setup directional light --- */

    NX_SetLightDirection(lights[NX_LIGHT_DIR], NX_VEC3(-1, -1, 0));
    NX_SetLightColor(lights[NX_LIGHT_DIR], NX_RED);
    NX_SetLightRange(lights[NX_LIGHT_DIR], 16.0f);
    NX_SetShadowActive(lights[NX_LIGHT_DIR], true);
    NX_SetLightActive(lights[NX_LIGHT_DIR], true);

    /* --- Setup spot light --- */

    NX_SetLightPosition(lights[NX_LIGHT_SPOT], NX_VEC3(0, 5, -10));
    NX_SetLightDirection(lights[NX_LIGHT_SPOT], NX_VEC3(0, -1, 1));
    NX_SetLightColor(lights[NX_LIGHT_SPOT], NX_GREEN);
    NX_SetLightRange(lights[NX_LIGHT_SPOT], 16.0f);
    NX_SetShadowActive(lights[NX_LIGHT_SPOT], true);
    NX_SetLightActive(lights[NX_LIGHT_SPOT], true);

    /* --- Setup omni light --- */

    NX_SetLightPosition(lights[NX_LIGHT_OMNI], NX_VEC3(0, 5, 10));
    NX_SetLightColor(lights[NX_LIGHT_OMNI], NX_BLUE);
    NX_SetLightRange(lights[NX_LIGHT_OMNI], 16.0f);
    NX_SetShadowActive(lights[NX_LIGHT_OMNI], true);
    NX_SetLightActive(lights[NX_LIGHT_OMNI], true);

    /* --- Setup environment --- */

    NX_Environment env = NX_GetDefaultEnvironment();
    env.background = NX_BLACK;
    env.ambient = NX_BLACK;

    NX_Camera camera = NX_GetDefaultCamera();

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 0, 0), 8.0f, 4.0f);

        for (int i = 0; i < NX_LIGHT_TYPE_COUNT; i++) {
            NX_BeginShadow3D(lights[i], &camera);
            DrawScene(ground, cube);
            NX_EndShadow3D();
        }

        NX_Begin3D(&camera, &env, NULL);
        {
            DrawScene(ground, cube);

            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            NX_Material mat = NX_GetDefaultMaterial();
            mat.shading = NX_SHADING_UNLIT;
            transform.scale = NX_VEC3_1(0.25f);

            mat.albedo.color = NX_GetLightColor(lights[NX_LIGHT_SPOT]);
            transform.translation = NX_GetLightPosition(lights[NX_LIGHT_SPOT]);
            NX_DrawMesh3D(cube, &mat, &transform);

            mat.albedo.color = NX_GetLightColor(lights[NX_LIGHT_OMNI]);
            transform.translation = NX_GetLightPosition(lights[NX_LIGHT_OMNI]);
            NX_DrawMesh3D(cube, &mat, &transform);
        }
        NX_End3D();
    }

    /* --- Cleanup --- */

    for (int i = 0; i < NX_LIGHT_TYPE_COUNT; i++) {
        NX_DestroyLight(lights[i]);
    }

    NX_DestroyMesh(ground);
    NX_DestroyMesh(cube);

    NX_Quit();

    return 0;
}
