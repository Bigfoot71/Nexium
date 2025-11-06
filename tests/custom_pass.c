/* custom_pass.c -- Test custom 2D shader pass applied on 3D scene rendering
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"

/* --- Shader uniform structure --- */

typedef struct {
    float scanlineDensity;
    float scanlineIntensity;
    float flickerSpeed;
    float vignetteStrength;
    float vignetteSoftness;
    float _padding[3];
} UniformScanline;

int main(void)
{
    /* --- Initialize engine and load resources --- */

    NX_Init("Nexium - Custom Pass", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    /* --- Create render target and shader --- */

    NX_RenderTexture* target = NX_CreateRenderTexture(800, 450);
    NX_Shader2D* shader = NX_LoadShader2D(NULL, "shaders/scanline.frag");

    NX_UpdateStaticShader2DBuffer(shader, 0, sizeof(UniformScanline), &(UniformScanline) {
        .scanlineDensity = 240.0f,
        .scanlineIntensity = 0.08f,
        .flickerSpeed = 1.5f,
        .vignetteStrength = 0.4f,
        .vignetteSoftness = 1.5f
    });

    /* --- Load scene resources --- */

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");

    /* --- Setup lighting --- */

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    /* --- Setup camera --- */

    NX_Camera camera = NX_GetDefaultCamera();

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        /* --- Update camera --- */

        CMN_UpdateCamera(&camera, NX_VEC3(0, 1, 0), 2.0f, 1.0f);

        /* --- Render 3D scene to texture --- */

        NX_Begin3D(&camera, NULL, target);
        NX_DrawMesh3D(ground, NULL, NULL);
        NX_DrawModel3D(model, NULL);
        NX_End3D();

        /* --- Post-process pass using custom shader --- */

        NX_Begin2D(NULL);
        NX_SetShader2D(shader);
        NX_SetTexture2D(NX_GetRenderTexture(target));
        NX_DrawRect2D(0, NX_GetWindowHeight(), NX_GetWindowWidth(), -NX_GetWindowHeight());
        NX_End2D();
    }

    /* --- Cleanup --- */

    NX_DestroyShader2D(shader);
    NX_DestroyRenderTexture(target);
    NX_DestroyMesh(ground);
    NX_DestroyModel(model);
    NX_DestroyLight(light);

    NX_Quit();

    return 0;
}
