#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_Init("Hyperion - Animation", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_Mesh* ground = HP_GenMeshQuad(HP_VEC2_1(10.0f), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Model* model = HP_LoadModel("models/CesiumMan.glb");

    int animCount = 0;
    HP_ModelAnimation** anim = HP_LoadModelAnimations("models/CesiumMan.glb", &animCount, 30);
    model->anim = anim[0];

    HP_Light* light = HP_CreateLight(HP_LIGHT_DIR);
    HP_SetLightDirection(light, HP_VEC3(-1, -1, -1));
    HP_SetShadowActive(light, true);
    HP_SetLightActive(light, true);

    HP_Camera camera = HP_GetDefaultCamera();

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3(0, 1, 0), 2.0f, 1.0f);
        model->animFrame += 40 * HP_GetFrameTime();

        HP_Transform groundPos = HP_TRANSFORM_IDENTITY;
        groundPos.translation.y = -0.5f;

        HP_Begin3D(&camera, NULL);
        HP_DrawMesh3D(ground, NULL, NULL);
        HP_DrawModel3D(model, NULL);
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
