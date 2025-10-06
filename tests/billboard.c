#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_Init("Hyperion - Animation", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_Mesh* ground = HP_GenMeshQuad(HP_VEC2_1(10.0f), HP_VEC2_ONE, HP_VEC3_UP);
    HP_Mesh* sprite = HP_GenMeshQuad(HP_VEC2_1(1.0f), HP_VEC2_ONE, HP_VEC3_BACK);

    HP_Material matSprite = HP_GetDefaultMaterial();
    matSprite.albedo.texture = HP_LoadTexture("images/spritesheet.png");
    HP_SetTextureWrap(matSprite.albedo.texture, HP_TEXTURE_WRAP_REPEAT);

    matSprite.billboard = HP_BILLBOARD_Y_AXIS;
    matSprite.blend = HP_BLEND_ALPHA;
    matSprite.cull = HP_CULL_NONE;

    HP_Light* light = HP_CreateLight(HP_LIGHT_DIR);
    HP_SetLightDirection(light, HP_VEC3(-1, -1, -1));
    HP_SetShadowActive(light, true);
    HP_SetLightActive(light, true);

    HP_Camera camera = HP_GetDefaultCamera();

    const int numFrame = 4;
    matSprite.texScale.x = 1.0f / numFrame;

    int frameCounter = 0;
    int animSpeed = 5;

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3(0, 0.5f, 0), 2.0f, 0.0f);

        if (HP_IsKeyJustPressed(HP_KEY_SPACE)) {
            matSprite.billboard = (matSprite.billboard + 1) % 3;
        }

        if (frameCounter % animSpeed == 0) {
            matSprite.texOffset.x = ((frameCounter / animSpeed) % numFrame) * matSprite.texScale.x;
        }
        frameCounter++;

        HP_Transform transform = HP_TRANSFORM_IDENTITY;

        HP_Begin3D(&camera, NULL, NULL);
        {
            transform.translation.y = -0.5f;
            HP_DrawMesh3D(ground, NULL, &transform);

            transform.translation.y = +0.5f;
            HP_DrawMesh3D(sprite, &matSprite, &transform);
        }
        HP_End3D();

        HP_Begin2D(NULL);
        {
            HP_SetColor2D(HP_BLACK);
            HP_DrawText2D(
                CMN_FormatText("BILLBOARD: %i\nPress SPACE to switch", matSprite.billboard),
                HP_VEC2_1(10), 32, HP_VEC2_ONE
            );
        }
        HP_End2D();
    }

    HP_Quit();

    return 0;
}
