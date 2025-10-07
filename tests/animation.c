#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Animation", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");

    int animCount = 0;
    NX_ModelAnimation** anim = NX_LoadModelAnimations("models/CesiumMan.glb", &animCount, 30);
    model->anim = anim[0];

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
        NX_DrawModel3D(model, NULL);
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
