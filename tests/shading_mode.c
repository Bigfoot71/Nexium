#include <Hyperion/Hyperion.h>
#include "./common.h"
#include "Hyperion/HP_Core.h"

int main(void)
{
    HP_AppDesc desc = {
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
        .flags = HP_FLAG_VSYNC_HINT
    };

    HP_InitEx("Hyperion - Shading Mode", 800, 450, &desc);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_Mesh* quad = HP_GenMeshQuad(HP_VEC2(100, 100), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_ONE, HP_VEC3_ONE);
    HP_Material material = HP_GetDefaultMaterial();

    HP_Light* light = HP_CreateLight(HP_LIGHT_SPOT);
    HP_SetLightPosition(light, HP_VEC3(-2, 5, 2));
    HP_SetLightDirection(light, HP_VEC3(1,-1,-1));
    HP_SetShadowActive(light, true);
    HP_SetLightActive(light, true);

    HP_Camera camera = HP_GetDefaultCamera();

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3_ZERO, 5.0f, 2.5f);

        if (HP_IsKeyJustPressed(HP_KEY_SPACE)) {
            material.shading = (material.shading + 1) % 3;
        }

        HP_Begin3D(&camera, NULL, NULL);
        {
            material.albedo.color = HP_GREEN;
            HP_DrawMesh3D(quad, &material, &(HP_Transform) {
                HP_VEC3(0, -0.501f, 0), HP_QUAT_IDENTITY, HP_VEC3_ONE
            });

            material.albedo.color = HP_BLUE;
            HP_DrawMesh3D(cube, &material, NULL);
        }
        HP_End3D();

        HP_Begin2D(NULL);
        {
            HP_SetColor2D(HP_BLACK);
            HP_DrawText2D("Press SPACE to change the shading mode", HP_VEC2_1(10), 24, HP_VEC2_ONE);
        }
        HP_End2D();
    }

    HP_Quit();

    return 0;
}
