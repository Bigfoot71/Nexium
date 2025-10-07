#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - SSAO", 800, 450, NX_FLAG_VSYNC_HINT);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_VEC3_ONE);

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    NX_Environment env = NX_GetDefaultEnvironment();
    env.ssao.enabled = true;

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 0, 0), 8.0f, 4.0f);

        if (NX_IsKeyJustPressed(NX_KEY_SPACE)) {
            env.ssao.enabled = !env.ssao.enabled;
        }

        NX_Begin3D(&camera, &env, NULL);
        {
            NX_DrawMesh3D(ground, NULL, NULL);

            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (float z = -4.5f; z <= 4.5f; z += 1.0f) {
                for (float x = -4.5f; x <= 4.5f; x += 1.0f) {
                    transform.translation = NX_VEC3(x, 0.25f, z);
                    NX_DrawMesh3D(cube, NULL, &transform);
                }
            }

        }
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
