#include <NX/Nexium.h>
#include "./common.h"

#define INSTANCE_COUNT 128

int main(void)
{
    NX_Init("Nexium - Instanced", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");

    int animCount = 0;
    NX_ModelAnimation** anim = NX_LoadModelAnimations("models/CesiumMan.glb", &animCount, 30);
    model->anim = anim[0];

    NX_Vec3 positions[INSTANCE_COUNT];
    NX_Color colors[INSTANCE_COUNT];

    for (int i = 0; i < INSTANCE_COUNT; i++) {
        positions[i] = NX_VEC3(
            NX_RandRangeFloat(NULL, -5, +5),
            0,
            NX_RandRangeFloat(NULL, -5, +5)
        );
        colors[i] = NX_ColorFromHSV(
            360 * NX_RandFloat(NULL),
            1, 1, 1
        );
    }

    NX_InstanceBuffer* instances = NX_CreateInstanceBuffer(NX_INSTANCE_DATA_POSITION | NX_INSTANCE_DATA_COLOR, INSTANCE_COUNT);
    NX_UpdateInstanceBuffer(instances, NX_INSTANCE_DATA_POSITION, 0, INSTANCE_COUNT, positions);
    NX_UpdateInstanceBuffer(instances, NX_INSTANCE_DATA_COLOR, 0, INSTANCE_COUNT, colors);

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 1, 0), 2.0f, 1.0f);
        model->animFrame += 40 * NX_GetFrameTime();

        NX_Begin3D(&camera, NULL, NULL);
        NX_DrawMesh3D(ground, NULL, NULL);
        NX_DrawModelInstanced3D(model, instances, INSTANCE_COUNT, NULL);
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
