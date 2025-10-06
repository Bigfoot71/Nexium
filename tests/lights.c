#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_AppDesc desc = {
        .render3D.resolution.x = 800,
        .render3D.resolution.y = 600,
        .render3D.sampleCount = 4,
        .targetFPS = 60
    };

    HP_InitEx("Hyperion - Lights", 800, 450, &desc);

    HP_Mesh* ground = HP_GenMeshQuad(HP_VEC2_1(100.0f), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_1(0.5f), HP_VEC3_ONE);
    HP_Mesh* sphere = HP_GenMeshSphere(0.1f, 16, 8);

    HP_Material matSphere = HP_GetDefaultMaterial();
    matSphere.shading = HP_SHADING_WIREFRAME;

    HP_Light* lights[128];
    for (int i = 0; i < HP_ARRAY_SIZE(lights); i++) {
        lights[i] = HP_CreateLight(HP_LIGHT_OMNI);
        HP_SetLightPosition(lights[i], HP_VEC3(
            HP_RandRangeFloat(NULL, -50, 50),
            HP_RandRangeFloat(NULL, 2, 5),
            HP_RandRangeFloat(NULL, -50, 50)
        ));
        HP_SetLightColor(lights[i], HP_ColorFromHSV(
            360 * HP_RandFloat(NULL), 1, 1, 1
        ));
        HP_SetLightActive(lights[i], true);
    }

    HP_Camera camera = HP_GetDefaultCamera();

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3(0, 0, 0), 16.0f, 8.0f);

        HP_Begin3D(&camera, NULL, NULL);
        {
            HP_DrawMesh3D(ground, NULL, NULL);

            HP_Transform transform = HP_TRANSFORM_IDENTITY;

            for (int z = -45; z <= 45; z++) {
                for (float x = -45; x <= 45; x++) {
                    transform.translation = HP_VEC3(x, 0.25f, z);
                    HP_DrawMesh3D(cube, NULL, &transform);
                }
            }

            for (int i = 0; i < HP_ARRAY_SIZE(lights); i++) {
                transform.translation = HP_GetLightPosition(lights[i]);
                matSphere.albedo.color = HP_GetLightColor(lights[i]);
                HP_DrawMesh3D(sphere, &matSphere, &transform);
            }
        }
        HP_End3D();

        HP_Begin2D(NULL);
        HP_SetColor2D(HP_BLACK);
        HP_DrawText2D(CMN_FormatText("FPS: %i", HP_GetFPS()), HP_VEC2(10, 10), 16, HP_VEC2_ONE);
        HP_End2D();
    }

    HP_Quit();

    return 0;
}
