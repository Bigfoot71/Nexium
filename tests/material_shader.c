#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Material Shader", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Shader3D* shader = NX_LoadMaterialShader("shaders/material.vert", "shaders/material.frag");

    NX_Image im0 = NX_GenImageChecked(64, 64, 8, 8, NX_WHITE, NX_BLANK);
    NX_Texture* tex0 = NX_CreateTextureFromImage(&im0);
    NX_DestroyImage(&im0);

    NX_Image im1 = NX_GenImageGradientSquare(64, 64, 0.8f, NX_WHITE, NX_BLANK);
    NX_Texture* tex1 = NX_CreateTextureFromImage(&im1);
    NX_DestroyImage(&im1);

    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_ONE, NX_IVEC3_ONE);
    NX_Material material = NX_GetDefaultMaterial();
    material.shader = shader;

    NX_Camera camera = NX_GetDefaultCamera();
    NX_Environment env = NX_GetDefaultEnvironment();
    env.bloom.mode = NX_BLOOM_ADDITIVE;
    env.bloom.strength = 0.01f;
    env.background = NX_BLACK;

    while (NX_FrameStep())
    {
        NX_UpdateStaticMaterialShaderBuffer(shader, 0, sizeof(NX_Vec4), &NX_VEC4(
            1.5f + sinf(4.0f * NX_GetElapsedTime()) * 0.5f, 0.0f, 0.0f, 0.0f
        ));

        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 8.0f, 4.0f);

        NX_Begin3D(&camera, &env, NULL);
        {
            NX_Transform T = NX_TRANSFORM_IDENTITY;

            NX_Color c0 = NX_ColorFromHSV(90.0 * NX_GetElapsedTime(), 1, 1, 1);
            NX_UpdateDynamicMaterialShaderBuffer(shader, sizeof(NX_Color), &c0);
            NX_SetMaterialShaderTexture(shader, 0, tex0);

            T.translation.x = -1.5f;
            NX_DrawMesh3D(cube, &material, &T);

            NX_Color c1 = NX_ColorFromHSV(90.0 * NX_GetElapsedTime() + 90.0f, 1, 1, 1);
            NX_UpdateDynamicMaterialShaderBuffer(shader, sizeof(NX_Color), &c1);
            NX_SetMaterialShaderTexture(shader, 0, tex1);

            T.translation.x = +1.5f;
            NX_DrawMesh3D(cube, &material, &T);
        }
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
