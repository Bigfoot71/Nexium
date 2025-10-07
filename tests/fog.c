#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Fog", 800, 450, NX_FLAG_VSYNC_HINT);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_VEC3_ONE);
    NX_Material mat = NX_GetDefaultMaterial();

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    NX_Environment env = NX_GetDefaultEnvironment();
    env.fog.mode = NX_FOG_LINEAR;
    env.fog.density = 0.9f;
    env.fog.start = 3.0f;
    env.fog.end = 30.0f;

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 0, 0), 8.0f, 4.0f);

        if (NX_IsKeyJustPressed(NX_KEY_SPACE)) {
            env.ssao.enabled = !env.ssao.enabled;
        }

        NX_Begin3D(&camera, &env, NULL);
        {
            mat.albedo.color = NX_GREEN;
            NX_DrawMesh3D(ground, &mat, NULL);

            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (float z = -4.5f; z <= 4.5f; z += 1.0f) {
                for (float x = -4.5f; x <= 4.5f; x += 1.0f) {
                    float tx = NX_Remap(x, -4.5f, 4.5f, 0.0f, 1.0f);
                    float tz = NX_Remap(z, -4.5f, 4.5f, 0.0f, 1.0f);
                    mat.albedo.color = NX_ColorFromHSV(NX_Lerp(0, 360, tx*tz), 1, 1, 1);
                    transform.translation = NX_VEC3(x, 0.25f, z);
                    NX_DrawMesh3D(cube, &mat, &transform);
                }
            }

        }
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
