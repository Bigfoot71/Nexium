#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_AppDesc desc = {
        .render3D.resolution.x = 800,
        .render3D.resolution.y = 600,
        .render3D.sampleCount = 4,
        .targetFPS = 60
    };

    NX_InitEx("Nexium - Lights", 800, 450, &desc);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(100.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_IVEC3_ONE);
    NX_Mesh* sphere = NX_GenMeshSphere(0.1f, 16, 8);

    NX_Material matSphere = NX_GetDefaultMaterial();
    matSphere.shading = NX_SHADING_UNLIT;

    NX_Light* lights[256];
    for (int i = 0; i < NX_ARRAY_SIZE(lights); i++) {
        lights[i] = NX_CreateLight(NX_LIGHT_OMNI);
        NX_SetLightPosition(lights[i], NX_VEC3(
            NX_RandRangeFloat(NULL, -50, 50),
            NX_RandRangeFloat(NULL, 2, 4),
            NX_RandRangeFloat(NULL, -50, 50)
        ));
        NX_SetLightColor(lights[i], NX_ColorFromHSV(
            360 * NX_RandFloat(NULL), 1, 1, 1
        ));
        NX_SetLightActive(lights[i], true);
    }

    NX_Camera camera = NX_GetDefaultCamera();

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 0, 0), 16.0f, 8.0f);

        NX_Begin3D(&camera, NULL, NULL);
        {
            NX_DrawMesh3D(ground, NULL, NULL);

            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (int z = -45; z <= 45; z++) {
                for (float x = -45; x <= 45; x++) {
                    transform.translation = NX_VEC3(x, 0.25f, z);
                    NX_DrawMesh3D(cube, NULL, &transform);
                }
            }

            for (int i = 0; i < NX_ARRAY_SIZE(lights); i++) {
                transform.translation = NX_GetLightPosition(lights[i]);
                matSphere.albedo.color = NX_GetLightColor(lights[i]);
                NX_DrawMesh3D(sphere, &matSphere, &transform);
            }
        }
        NX_End3D();

        NX_Begin2D(NULL);
        NX_SetColor2D(NX_BLACK);
        NX_DrawText2D(CMN_FormatText("FPS: %i", NX_GetFPS()), NX_VEC2(10, 10), 16, NX_VEC2_ONE);
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
