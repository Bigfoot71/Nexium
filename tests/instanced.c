#include <Hyperion/Hyperion.h>
#include "./common.h"

#define INSTANCE_COUNT 128

int main(void)
{
    HP_Init("Hyperion - Instanced", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_Mesh* ground = HP_GenMeshQuad(HP_VEC2_1(10.0f), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Model* model = HP_LoadModel("models/CesiumMan.glb");

    int animCount = 0;
    HP_ModelAnimation** anim = HP_LoadModelAnimations("models/CesiumMan.glb", &animCount, 30);
    model->anim = anim[0];

    HP_Mat4 matrices[INSTANCE_COUNT];
    HP_Color colors[INSTANCE_COUNT];

    for (int i = 0; i < INSTANCE_COUNT; i++) {
        matrices[i] = HP_Mat4Translate(HP_VEC3(
            HP_RandRangeFloat(NULL, -5, +5),
            0,
            HP_RandRangeFloat(NULL, -5, +5)
        ));
        colors[i] = HP_ColorFromHSV(
            360 * HP_RandFloat(NULL),
            1, 1, 1
        );
    }

    HP_InstanceBuffer* instances = HP_CreateInstanceBuffer(HP_INSTANCE_DATA_MATRIX | HP_INSTANCE_DATA_COLOR, INSTANCE_COUNT);
    HP_UpdateInstanceBuffer(instances, HP_INSTANCE_DATA_MATRIX, matrices, 0, INSTANCE_COUNT, false);
    HP_UpdateInstanceBuffer(instances, HP_INSTANCE_DATA_COLOR, colors, 0, INSTANCE_COUNT, false);

    HP_Light* light = HP_CreateLight(HP_LIGHT_DIR);
    HP_SetLightDirection(light, HP_VEC3(-1, -1, -1));
    HP_SetShadowActive(light, true);
    HP_SetLightActive(light, true);

    HP_Camera camera = HP_GetDefaultCamera();

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3(0, 1, 0), 2.0f, 1.0f);
        model->animFrame += 40 * HP_GetFrameTime();

        HP_Begin3D(&camera, NULL, NULL);
        HP_DrawMesh3D(ground, NULL, NULL);
        HP_DrawModelInstanced3D(model, instances, INSTANCE_COUNT, NULL);
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
