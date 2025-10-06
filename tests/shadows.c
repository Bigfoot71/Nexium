#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_Init("Hyperion - Shadows", 800, 450, HP_FLAG_VSYNC_HINT);

    HP_Mesh* ground = HP_GenMeshQuad(HP_VEC2_1(10.0f), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_1(0.5f), HP_VEC3_ONE);

    HP_Light* dirLight = HP_CreateLight(HP_LIGHT_DIR);
    HP_SetLightDirection(dirLight, HP_VEC3(-1, -1, 0));
    HP_SetLightColor(dirLight, HP_RED);
    HP_SetShadowActive(dirLight, true);
    HP_SetLightActive(dirLight, true);

    HP_Light* spotLight = HP_CreateLight(HP_LIGHT_SPOT);
    HP_SetLightPosition(spotLight, HP_VEC3(0, 5, -10));
    HP_SetLightDirection(spotLight, HP_VEC3(0, -1, 1));
    HP_SetLightColor(spotLight, HP_GREEN);
    HP_SetShadowActive(spotLight, true);
    HP_SetLightActive(spotLight, true);

    HP_Light* omniLight = HP_CreateLight(HP_LIGHT_OMNI);
    HP_SetLightPosition(omniLight, HP_VEC3(0, 5, 10));
    HP_SetLightColor(omniLight, HP_BLUE);
    HP_SetShadowActive(omniLight, true);
    HP_SetLightActive(omniLight, true);

    HP_Environment env = HP_GetDefaultEnvironment();
    env.background = HP_BLACK;
    env.ambient = HP_BLACK;

    HP_Camera camera = HP_GetDefaultCamera();

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3(0, 0, 0), 8.0f, 4.0f);

        HP_Begin3D(&camera, &env, NULL);
        {
            HP_DrawMesh3D(ground, NULL, NULL);

            HP_Transform transform = HP_TRANSFORM_IDENTITY;

            for (float z = -4.0f; z <= 4.0f; z += 2.0f) {
                for (float x = -4.0f; x <= 4.0f; x += 2.0f) {
                    transform.translation = HP_VEC3(x, 0.25f, z);
                    HP_DrawMesh3D(cube, NULL, &transform);
                }
            }

            /* --- Draw spot/omni lights --- */

            HP_Material mat = HP_GetDefaultMaterial();
            mat.shading = HP_SHADING_UNLIT;

            transform.scale = HP_VEC3_1(0.25f);

            mat.albedo.color = HP_GetLightColor(spotLight);
            transform.translation = HP_GetLightPosition(spotLight);
            HP_DrawMesh3D(cube, &mat, &transform);

            mat.albedo.color = HP_GetLightColor(omniLight);
            transform.translation = HP_GetLightPosition(omniLight);
            HP_DrawMesh3D(cube, &mat, &transform);
        }
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
