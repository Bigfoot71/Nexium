/* animation.c -- Test animated models with or without instancing and their shadows
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"

#define MAX_INSTANCE 256

int main(void)
{
    /* --- Initialize engine and load resources --- */

    NX_Init("Nexium - Animation", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    /* --- Create ground plane --- */

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(100.0f), NX_IVEC2_ONE, NX_VEC3_UP);

    /* --- Load model and animation --- */

    NX_AnimationLib* animLib = NX_LoadAnimationLib("models/CesiumMan.glb", 30);
    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");

    /* --- Create and assign animation player --- */

    NX_AnimationPlayer* animPlayer = NX_CreateAnimationPlayer(model->skeleton, animLib);
    animPlayer->states[0] = (NX_AnimationState) { .weight = 1.0f, .loop = true };
    model->player = animPlayer;

    /* --- Create instance buffer and randomize positions --- */

    NX_InstanceBuffer* instances = NX_CreateInstanceBuffer(NX_INSTANCE_POSITION, MAX_INSTANCE);
    NX_Vec3* positions = NX_MapInstanceBuffer(instances, NX_INSTANCE_POSITION);

    positions[0] = NX_VEC3_ZERO;
    for (int i = 1; i < MAX_INSTANCE; i++) {
        positions[i] = NX_VEC3(
            NX_RandRangeFloat(NULL, -50.0f, 50.0f),
            0,
            NX_RandRangeFloat(NULL, -50.0f, 50.0f)
        );
    }

    NX_UnmapInstanceBuffer(instances, NX_INSTANCE_POSITION);

    /* --- Setup directional light and shadows --- */

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    /* --- Setup camera --- */

    NX_Camera camera = NX_GetDefaultCamera();
    int instanceCount = 1;

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        /* --- Update camera and animation --- */

        CMN_UpdateCamera(&camera, NX_VEC3(0, 1, 0), 2.0f, 1.0f);
        instanceCount = NX_CLAMP(instanceCount + NX_GetMouseWheel().y, 1, MAX_INSTANCE);
        NX_UpdateAnimationPlayer(animPlayer, NX_GetDeltaTime());

        /* --- 3D rendering --- */

        NX_Begin3D(&camera, NULL, NULL);
        NX_DrawMesh3D(ground, NULL, NULL);
        (instanceCount <= 1)
            ? NX_DrawModel3D(model, NULL)
            : NX_DrawModelInstanced3D(model, instances, instanceCount, NULL);
        NX_End3D();

        /* --- 2D UI rendering --- */

        NX_Begin2D(NULL);
        NX_SetColor2D(NX_YELLOW);
        NX_DrawText2D(CMN_FormatText("Instances: %i", instanceCount), NX_VEC2(10, 10), 16, NX_VEC2_ONE);
        NX_End2D();
    }

    /* --- Cleanup --- */

    NX_DestroyAnimationPlayer(animPlayer);
    NX_DestroyInstanceBuffer(instances);
    NX_DestroyAnimationLib(animLib);
    NX_DestroyMesh(ground);
    NX_DestroyLight(light);
    NX_DestroyModel(model);

    NX_Quit();

    return 0;
}
