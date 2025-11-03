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
    NX_Init("Nexium - Animation", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(100.0f), NX_IVEC2_ONE, NX_VEC3_UP);

    int animCount = 0;
    NX_Animation** anims = NX_LoadAnimations("models/CesiumMan.glb", &animCount, 30);
    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");
    model->anim = anims[0];

    NX_InstanceBuffer* instances = NX_CreateInstanceBuffer(NX_INSTANCE_POSITION, MAX_INSTANCE);
    NX_Vec3* iPositions = NX_MapInstanceBuffer(instances, NX_INSTANCE_POSITION);
    iPositions[0] = NX_VEC3_ZERO;
    for (int i = 1; i < MAX_INSTANCE; i++) {
        iPositions[i] = NX_VEC3(
            NX_RandRangeFloat(NULL, -50.0f, 50.0f),
            0,
            NX_RandRangeFloat(NULL, -50.0f, 50.0f)
        );
    }
    NX_UnmapInstanceBuffer(instances, NX_INSTANCE_POSITION);

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    int instanceCount = 1;

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 1, 0), 2.0f, 1.0f);
        instanceCount = NX_CLAMP(instanceCount + NX_GetMouseWheel().y, 1, MAX_INSTANCE);
        model->animFrame += 40 * NX_GetDeltaTime();

        NX_Begin3D(&camera, NULL, NULL);
        {
            NX_DrawMesh3D(ground, NULL, NULL);
            if (instanceCount <= 1) NX_DrawModel3D(model, NULL);
            else NX_DrawModelInstanced3D(model, instances, instanceCount, NULL);
        }
        NX_End3D();

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_YELLOW);
            NX_DrawText2D(CMN_FormatText("Instances: %i", instanceCount), NX_VEC2(10, 10), 16, NX_VEC2_ONE);
        }
        NX_End2D();
    }

    NX_DestroyAnimations(anims, animCount);
    NX_DestroyInstanceBuffer(instances);
    NX_DestroyMesh(ground);
    NX_DestroyModel(model);

    NX_Quit();

    return 0;
}
