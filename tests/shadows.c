#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Shadows", 800, 450, NX_FLAG_VSYNC_HINT);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_VEC3_ONE);

    NX_Light* dirLight = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(dirLight, NX_VEC3(-1, -1, 0));
    NX_SetLightColor(dirLight, NX_RED);
    NX_SetShadowActive(dirLight, true);
    NX_SetLightActive(dirLight, true);

    NX_Light* spotLight = NX_CreateLight(NX_LIGHT_SPOT);
    NX_SetLightPosition(spotLight, NX_VEC3(0, 5, -10));
    NX_SetLightDirection(spotLight, NX_VEC3(0, -1, 1));
    NX_SetLightColor(spotLight, NX_GREEN);
    NX_SetShadowActive(spotLight, true);
    NX_SetLightActive(spotLight, true);

    NX_Light* omniLight = NX_CreateLight(NX_LIGHT_OMNI);
    NX_SetLightPosition(omniLight, NX_VEC3(0, 5, 10));
    NX_SetLightColor(omniLight, NX_BLUE);
    NX_SetShadowActive(omniLight, true);
    NX_SetLightActive(omniLight, true);

    NX_Environment env = NX_GetDefaultEnvironment();
    env.background = NX_BLACK;
    env.ambient = NX_BLACK;

    NX_Camera camera = NX_GetDefaultCamera();

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 0, 0), 8.0f, 4.0f);

        NX_Begin3D(&camera, &env, NULL);
        {
            NX_DrawMesh3D(ground, NULL, NULL);

            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (float z = -4.0f; z <= 4.0f; z += 2.0f) {
                for (float x = -4.0f; x <= 4.0f; x += 2.0f) {
                    transform.translation = NX_VEC3(x, 0.25f, z);
                    NX_DrawMesh3D(cube, NULL, &transform);
                }
            }

            /* --- Draw spot/omni lights --- */

            NX_Material mat = NX_GetDefaultMaterial();
            mat.shading = NX_SHADING_UNLIT;

            transform.scale = NX_VEC3_1(0.25f);

            mat.albedo.color = NX_GetLightColor(spotLight);
            transform.translation = NX_GetLightPosition(spotLight);
            NX_DrawMesh3D(cube, &mat, &transform);

            mat.albedo.color = NX_GetLightColor(omniLight);
            transform.translation = NX_GetLightPosition(omniLight);
            NX_DrawMesh3D(cube, &mat, &transform);
        }
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
