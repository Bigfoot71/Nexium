/* billboard.c -- Test billboard materials and sprite animation
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    /* --- Initialize engine and load resources --- */

    NX_Init("Nexium - Billboard", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    /* --- Create meshes --- */

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Mesh* sprite = NX_GenMeshQuad(NX_VEC2_1(1.0f), NX_IVEC2_ONE, NX_VEC3_BACK);

    /* --- Setup sprite material --- */

    NX_Material matSprite = NX_GetDefaultMaterial();
    matSprite.albedo.texture = NX_LoadTexture("images/spritesheet.png");
    NX_SetTextureWrap(matSprite.albedo.texture, NX_TEXTURE_WRAP_REPEAT);

    matSprite.billboard = NX_BILLBOARD_Y_AXIS;
    matSprite.blend = NX_BLEND_ALPHA;
    matSprite.cull = NX_CULL_NONE;

    /* --- Setup directional light --- */

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    /* --- Setup camera and animation parameters --- */

    NX_Camera camera = NX_GetDefaultCamera();

    const int numFrame = 4;
    matSprite.texScale.x = 1.0f / numFrame;

    int frameCounter = 0;
    const int animSpeed = 5;

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        /* --- Update camera --- */

        CMN_UpdateCamera(&camera, NX_VEC3(0, 0.5f, 0), 2.0f, 0.0f);

        /* --- Handle billboard mode switching --- */

        if (NX_IsKeyJustPressed(NX_KEY_SPACE))
            matSprite.billboard = (matSprite.billboard + 1) % 3;

        /* --- Animate texture offset --- */

        if (frameCounter % animSpeed == 0) {
            matSprite.texOffset.x = ((frameCounter / animSpeed) % numFrame) * matSprite.texScale.x;
        }
        frameCounter++;

        /* --- 3D rendering --- */

        NX_Transform transform = NX_TRANSFORM_IDENTITY;

        NX_BeginShadow3D(light, &camera);
        {
            transform.translation.y = -0.5f;
            NX_DrawMesh3D(ground, NULL, &transform);

            transform.translation.y = +0.5f;
            NX_DrawMesh3D(sprite, &matSprite, &transform);
        }
        NX_EndShadow3D();

        NX_Begin3D(&camera, NULL, NULL);
        {
            transform.translation.y = -0.5f;
            NX_DrawMesh3D(ground, NULL, &transform);

            transform.translation.y = +0.5f;
            NX_DrawMesh3D(sprite, &matSprite, &transform);
        }
        NX_End3D();

        /* --- 2D UI rendering --- */

        NX_Begin2D(NULL);
        {
            NX_SetColor2D(NX_BLACK);
            NX_DrawText2D(
                CMN_FormatText("BILLBOARD: %i\nPress SPACE to switch", matSprite.billboard),
                NX_VEC2_1(10), 32, NX_VEC2_ONE
            );
        }
        NX_End2D();
    }

    /* --- Cleanup --- */

    NX_DestroyMaterialResources(&matSprite);
    NX_DestroyMesh(ground);
    NX_DestroyMesh(sprite);
    NX_DestroyLight(light);

    NX_Quit();

    return 0;
}
