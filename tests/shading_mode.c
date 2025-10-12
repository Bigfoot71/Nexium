#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_AppDesc desc = {
        .render2D = {
            .sampleCount = 4,
            .resolution.x = 800,
            .resolution.y = 450
        },
        .render3D = {
            .sampleCount = 4,
            .resolution.x = 800,
            .resolution.y = 450
        },
        .flags = NX_FLAG_VSYNC_HINT
    };

    NX_InitEx("Nexium - Shading Mode", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Mesh* quad = NX_GenMeshQuad(NX_VEC2(100, 100), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_ONE, NX_VEC3_ONE);
    NX_Material material = NX_GetDefaultMaterial();

    NX_Light* light = NX_CreateLight(NX_LIGHT_SPOT);
    NX_SetLightPosition(light, NX_VEC3(-2, 5, 2));
    NX_SetLightDirection(light, NX_VEC3(1,-1,-1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 5.0f, 2.5f);

        if (NX_IsKeyJustPressed(NX_KEY_SPACE)) {
            material.shading = (material.shading + 1) % 2;
        }

        NX_Begin3D(&camera, NULL, NULL);
        {
            material.albedo.color = NX_GREEN;
            NX_DrawMesh3D(quad, &material, &(NX_Transform) {
                NX_VEC3(0, -0.501f, 0), NX_QUAT_IDENTITY, NX_VEC3_ONE
            });

            material.albedo.color = NX_BLUE;
            NX_DrawMesh3D(cube, &material, NULL);
        }
        NX_End3D();

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_BLACK);
            NX_DrawText2D("Press SPACE to change the shading mode", NX_VEC2_1(10), 24, NX_VEC2_ONE);
        }
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
