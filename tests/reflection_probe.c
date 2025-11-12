/* reflection_pass.c -- Test reflection probe with sky reflections
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
    /* --- Initialize engine and load resources --- */

    NX_Init("Nexium - Reflection Probe", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    /* --- Load scene resources --- */

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Mesh* sphere = NX_GenMeshSphere(0.5f, 32, 32);

    NX_Material matSphere = NX_GetDefaultMaterial();
    matSphere.orm.roughness = 0.0f;
    matSphere.orm.metalness = 1.0f;

    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");

    /* --- Setup scene environment --- */

    NX_Cubemap* skyCubemap = NX_LoadCubemap("cubemaps/panorama.hdr");
    NX_IndirectLight* skyLight = NX_CreateIndirectLight(skyCubemap);

    NX_Environment env = NX_GetDefaultEnvironment();
    env.sky.cubemap = skyCubemap;
    env.sky.light = skyLight;
    env.sky.intensity = 0.5f;

    /* --- Setup camera --- */

    NX_Camera camera = NX_GetDefaultCamera();

    /* --- Setup lighting --- */

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_BeginShadow3D(light, &camera, 0);
    NX_DrawMesh3D(ground, NULL, NULL);
    NX_DrawModel3D(model, NULL);
    NX_EndShadow3D();

    /* --- Setup reflection probe --- */

    NX_Cubemap* probeCubemap = NX_CreateCubemap(512, NX_PIXEL_FORMAT_RGB16F);
    NX_Probe probe = { .position = NX_VEC3(-2, 2, 2), 8.0f, .cullMask = NX_LAYER_ALL };

    NX_BeginCubemap3D(probeCubemap, &probe, &env, 0);
    NX_DrawMesh3D(ground, NULL, NULL);
    NX_DrawModel3D(model, NULL);
    NX_EndCubemap3D();

    NX_IndirectLight* reflection = NX_CreateIndirectLight(probeCubemap);

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 1, 0), 2.0f, 1.0f);

        NX_Transform transSphere = NX_TRANSFORM_IDENTITY;
        transSphere.translation = probe.position;

        NX_Begin3D(&camera, &env, 0);
        NX_DrawReflectionProbe3D(reflection, &probe);
        NX_DrawMesh3D(ground, NULL, NULL);
        NX_DrawMesh3D(sphere, &matSphere, &transSphere);
        NX_DrawModel3D(model, NULL);
        NX_End3D();
    }

    /* --- Cleanup --- */

    NX_DestroyIndirectLight(reflection);
    NX_DestroyIndirectLight(skyLight);
    NX_DestroyCubemap(probeCubemap);
    NX_DestroyCubemap(skyCubemap);
    NX_DestroyMesh(ground);
    NX_DestroyMesh(sphere);
    NX_DestroyModel(model);
    NX_DestroyLight(light);

    NX_Quit();

    return 0;
}
