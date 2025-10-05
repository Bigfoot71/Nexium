#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_Init("Hyperion - Material Shader", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_MaterialShader* shader = HP_LoadMaterialShader("shaders/material.vert", "shaders/material.frag");

    HP_Image im0 = HP_GenImageChecked(64, 64, 8, 8, HP_WHITE, HP_BLANK);
    HP_Texture* tex0 = HP_CreateTexture(&im0);
    HP_DestroyImage(&im0);

    HP_Image im1 = HP_GenImageGradientSquare(64, 64, 0.8f, HP_WHITE, HP_BLANK);
    HP_Texture* tex1 = HP_CreateTexture(&im1);
    HP_DestroyImage(&im1);

    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_ONE, HP_VEC3_ONE);
    HP_Material material = HP_GetDefaultMaterial();
    material.shader = shader;

    HP_Camera camera = HP_GetDefaultCamera();
    HP_Environment env = HP_GetDefaultEnvironment();
    env.bloom.mode = HP_BLOOM_ADDITIVE;
    env.bloom.strength = 0.01f;
    env.background = HP_BLACK;

    while (HP_FrameStep())
    {
        HP_UpdateStaticMaterialBuffer(shader, 0, sizeof(HP_Vec4), &HP_VEC4(
            1.5f + sinf(4.0f * HP_GetElapsedTime()) * 0.5f, 0.0f, 0.0f, 0.0f
        ));

        CMN_UpdateCamera(&camera, HP_VEC3_ZERO, 8.0f, 4.0f);

        HP_Begin3D(&camera, &env, NULL);
        {
            HP_Transform T = HP_TRANSFORM_IDENTITY;

            HP_Color c0 = HP_ColorFromHSV(90.0 * HP_GetElapsedTime(), 1, 1, 1);
            HP_UpdateDynamicMaterialBuffer(shader, sizeof(HP_Color), &c0);
            HP_SetMaterialShaderTexture(shader, 0, tex0);

            T.translation.x = -1.5f;
            HP_DrawMesh3D(cube, &material, &T);

            HP_Color c1 = HP_ColorFromHSV(90.0 * HP_GetElapsedTime() + 90.0f, 1, 1, 1);
            HP_UpdateDynamicMaterialBuffer(shader, sizeof(HP_Color), &c1);
            HP_SetMaterialShaderTexture(shader, 0, tex1);

            T.translation.x = +1.5f;
            HP_DrawMesh3D(cube, &material, &T);
        }
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
