/* pbr.c -- Test PBR model rendering with multiple environment settings
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
        .render3D.resolution = { 800, 450 },
        .render3D.sampleCount = 4,
        .targetFPS = 60
    };

    NX_InitEx("Nexium - PBR", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_SetDefaultTextureFilter(NX_TEXTURE_FILTER_TRILINEAR);
    NX_SetDefaultTextureAnisotropy(4.0f);

    /* --- Load model and environment resources --- */

    NX_Model* model = NX_LoadModel("models/DamagedHelmet.glb");

    NX_Cubemap* skybox = NX_LoadCubemap("cubemaps/panorama.hdr");
    NX_ReflectionProbe* skyprobe = NX_CreateReflectionProbe(skybox);

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    /* --- Setup two different environments --- */

    NX_Environment envs[2] = {0};

    /* Simple ambient environment */

    envs[0] = NX_GetDefaultEnvironment();
    envs[0].ambient = NX_COLOR_1(0.1f);
    envs[0].background = NX_BLACK;

    /* Advanced environment */

    envs[1] = NX_GetDefaultEnvironment();
    envs[1].bloom.mode = NX_BLOOM_MIX;
    envs[1].bloom.strength = 0.08f;
    envs[1].tonemap.mode = NX_TONEMAP_ACES;
    envs[1].tonemap.exposure = 2.0f;
    envs[1].tonemap.white = 8.0f;
    envs[1].sky.intensity = 0.2f;
    envs[1].sky.cubemap = skybox;
    envs[1].sky.probe = skyprobe;

    int envIndex = 0;

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        /* Switch environment on SPACE key */
        if (NX_IsKeyJustPressed(NX_KEY_SPACE)) {
            envIndex = (envIndex + 1) % NX_ARRAY_SIZE(envs);
        }

        /* Update camera */
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 2.5f, 1.0f);

        /* --- 3D rendering --- */

        NX_Begin3D(&camera, &envs[envIndex], NULL);
        NX_DrawModel3D(model, NULL);
        NX_End3D();

        /* --- 2D UI --- */

        NX_Begin2D(NULL);
        NX_DrawText2D("Press SPACE to change environment", NX_VEC2_1(10), 32, NX_VEC2_ONE);
        NX_End2D();
    }

    /* --- Cleanup --- */

    NX_DestroyModel(model);
    NX_DestroyCubemap(skybox);
    NX_DestroyReflectionProbe(skyprobe);
    NX_DestroyLight(light);

    NX_Quit();

    return 0;
}
