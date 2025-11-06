/* skybox.c -- Tests skyboxes loaded from a file or procedurally generated, as well as the IBL
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"

NX_Cubemap* GenerateSkybox(int size)
{
    NX_Cubemap* skybox = NX_CreateCubemap(1024, NX_PIXEL_FORMAT_RGB16F);
    NX_GenerateSkybox(skybox, &(NX_Skybox) {
        .sunDirection = NX_VEC3(-1, -1, -1),
        .skyColorTop = NX_COLOR(0.38f, 0.45f, 0.55f, 1.0f),
        .skyColorHorizon = NX_COLOR(0.6f, 0.75f, 0.9f, 1.0f),
        .sunColor = NX_COLOR(1.0f, 0.95f, 0.8f, 1.0f),
        .groundColor = NX_COLOR(0.2f, 0.17f, 0.13f, 1.0f),
        .sunSize = 0.02f,
        .haze = 0.1f,
        .energy = 1.0f
    });
    return skybox;
}

int main(void)
{
    NX_Init("Nexium - Skybox", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Mesh* sphere = NX_GenMeshSphere(0.4f, 32, 16);
    NX_Material material = NX_GetDefaultMaterial();
    material.albedo.color = NX_SILVER;

    NX_Environment env[2];
    for (int i = 0; i < NX_ARRAY_SIZE(env); i++) {
        env[i] = NX_GetDefaultEnvironment();
        env[i].tonemap.mode = NX_TONEMAP_ACES;
        env[i].tonemap.exposure = 4.0f;
        env[i].tonemap.white = 8.0f;
        env[i].sky.intensity = 0.2f;
    }

    env[0].sky.cubemap = NX_LoadCubemap("cubemaps/panorama.hdr");
    env[0].sky.probe = NX_CreateReflectionProbe(env[0].sky.cubemap);

    env[1].sky.cubemap = GenerateSkybox(1024);
    env[1].sky.probe = NX_CreateReflectionProbe(env[1].sky.cubemap);

    NX_Camera camera = NX_GetDefaultCamera();

    int currentEnv = 0;

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 16.0f, 0.0f);

        if (NX_IsKeyJustPressed(NX_KEY_SPACE)) {
            currentEnv = (currentEnv + 1) % NX_ARRAY_SIZE(env);
        }

        NX_Begin3D(&camera, &env[currentEnv], NULL);
        {
            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (int x = -5; x <= 5; x++) {
                for (int y = -5; y <= 5; y++) {
                    transform.translation.x = x;
                    transform.translation.y = y;
                    material.orm.roughness = NX_Remap(x, -5, 5, 1, 0);
                    material.orm.metalness = NX_Remap(y, -5, 5, 0, 1);
                    NX_DrawMesh3D(sphere, &material, &transform);
                }
            }
        }
        NX_End3D();

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_YELLOW);
            NX_DrawText2D("Press SPACE to change skybox", NX_VEC2_1(10), 16, NX_VEC2_ONE);
        }
        NX_End2D();
    }

    NX_DestroyReflectionProbe(env[0].sky.probe);
    NX_DestroyReflectionProbe(env[1].sky.probe);
    NX_DestroyCubemap(env[0].sky.cubemap);
    NX_DestroyCubemap(env[1].sky.cubemap);
    NX_DestroyMesh(sphere);

    NX_Quit();

    return 0;
}
