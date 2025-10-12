/* post_process.c -- Dirty quick test of fog, SSAO and bloom
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

// TODO: Need a thorough test of each effect as well as tonemapping and color adjustments

#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Post Process", 800, 450, NX_FLAG_VSYNC_HINT);

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_VEC2_ONE, NX_VEC3_UP);
    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_1(0.5f), NX_VEC3_ONE);
    NX_Material mat = NX_GetDefaultMaterial();

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    NX_Environment env = NX_GetDefaultEnvironment();

    env.background = NX_BLACK;

    env.fog.mode = NX_FOG_DISABLED;
    env.fog.density = 0.9f;
    env.fog.start = 3.0f;
    env.fog.end = 30.0f;

    env.ssao.enabled = false;
    env.ssao.power = 2.0f;

    env.bloom.mode = NX_BLOOM_DISABLED;
    env.bloom.strength = 0.01f;

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 0, 0), 8.0f, 4.0f);

        if (NX_IsKeyJustPressed(NX_KEY_1)) {
            env.fog.mode = (env.fog.mode == NX_FOG_DISABLED) ?  NX_FOG_LINEAR : NX_FOG_DISABLED;
        }
        if (NX_IsKeyJustPressed(NX_KEY_2)) {
            env.ssao.enabled = !env.ssao.enabled;
        }
        if (NX_IsKeyJustPressed(NX_KEY_3)) {
            env.bloom.mode = (env.bloom.mode == NX_BLOOM_DISABLED) ? NX_BLOOM_ADDITIVE : NX_BLOOM_DISABLED;
        }

        NX_Begin3D(&camera, &env, NULL);
        {
            mat.emission.energy = 0.0f;
            mat.albedo.color = NX_GREEN;
            NX_DrawMesh3D(ground, &mat, NULL);

            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (float z = -4.5f; z <= 4.5f; z += 1.0f) {
                for (float x = -4.5f; x <= 4.5f; x += 1.0f)
                {
                    float tx = NX_Remap(x, -4.5f, 4.5f, 0.0f, 1.0f);
                    float tz = NX_Remap(z, -4.5f, 4.5f, 0.0f, 1.0f);
                    NX_Color color = NX_ColorFromHSV(NX_Lerp(0, 360, tx*tz), 1, 1, 1);

                    if (env.bloom.mode == NX_BLOOM_DISABLED) {
                        mat.emission.color = NX_BLACK;
                        mat.emission.energy = 0.0f;
                        mat.albedo.color = color;
                    }
                    else {
                        mat.emission.color = color;
                        mat.emission.energy = 1.0f;
                        mat.albedo.color = NX_BLACK;
                    }

                    transform.translation = NX_VEC3(x, 0.25f, z);
                    NX_DrawMesh3D(cube, &mat, &transform);
                }
            }
        }
        NX_End3D();

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_YELLOW);
            NX_DrawText2D(
                CMN_FormatText(
                    "FOG: %s - Key 1\nSSAO: %s - Key 2\nBLOOM: %s - Key 3",
                    (env.fog.mode == NX_FOG_DISABLED) ? "OFF" : "ON",
                    env.ssao.enabled ? "ON" : "OFF",
                    (env.bloom.mode == NX_BLOOM_DISABLED) ? "OFF" : "ON"
                ),
                NX_VEC2_1(10), 24, NX_VEC2_ONE
            );
        }
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
