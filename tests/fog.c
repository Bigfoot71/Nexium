#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_Init("Hyperion - Fog", 800, 450, HP_FLAG_VSYNC_HINT);

    HP_Mesh* ground = HP_GenMeshQuad(HP_VEC2_1(10.0f), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_1(0.5f), HP_VEC3_ONE);
    HP_Material mat = HP_GetDefaultMaterial();

    HP_Light* light = HP_CreateLight(HP_LIGHT_DIR);
    HP_SetLightDirection(light, HP_VEC3(-1, -1, -1));
    HP_SetShadowActive(light, true);
    HP_SetLightActive(light, true);

    HP_Camera camera = HP_GetDefaultCamera();

    HP_Environment env = HP_GetDefaultEnvironment();
    env.fog.mode = HP_FOG_LINEAR;
    env.fog.density = 0.9f;
    env.fog.start = 3.0f;
    env.fog.end = 30.0f;

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3(0, 0, 0), 8.0f, 4.0f);

        if (HP_IsKeyJustPressed(HP_KEY_SPACE)) {
            env.ssao.enabled = !env.ssao.enabled;
        }

        HP_Begin3D(&camera, &env, NULL);
        {
            mat.albedo.color = HP_GREEN;
            HP_DrawMesh3D(ground, &mat, NULL);

            HP_Transform transform = HP_TRANSFORM_IDENTITY;

            for (float z = -4.5f; z <= 4.5f; z += 1.0f) {
                for (float x = -4.5f; x <= 4.5f; x += 1.0f) {
                    float tx = HP_Remap(x, -4.5f, 4.5f, 0.0f, 1.0f);
                    float tz = HP_Remap(z, -4.5f, 4.5f, 0.0f, 1.0f);
                    mat.albedo.color = HP_ColorFromHSV(HP_Lerp(0, 360, tx*tz), 1, 1, 1);
                    transform.translation = HP_VEC3(x, 0.25f, z);
                    HP_DrawMesh3D(cube, &mat, &transform);
                }
            }

        }
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
