#include <NX/Nexium.h>
#include "./common.h"

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
    NX_Init("Nexium - Custom Pass", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_RenderTexture* target = NX_CreateRenderTexture(1920, 1080);
    NX_Shader* shader = NX_LoadShader(NULL, "shaders/scanline.frag");

    NX_UpdateStaticShaderBuffer(shader, 0, sizeof(UniformScanline), &(UniformScanline) {
        .scanlineDensity = 240.0,
        .scanlineIntensity = 0.08,
        .flickerSpeed = 1.5,
        .vignetteStrength = 0.4,
        .vignetteSoftness = 1.5
    });

    NX_Mesh* ground = NX_GenMeshQuad(NX_VEC2_1(10.0f), NX_IVEC2_ONE, NX_VEC3_UP);
    NX_Model* model = NX_LoadModel("models/CesiumMan.glb");

    int animCount = 0;
    NX_ModelAnimation** anim = NX_LoadModelAnimations("models/CesiumMan.glb", &animCount, 30);
    model->anim = anim[0];

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3(0, 1, 0), 2.0f, 1.0f);
        model->animFrame += 40 * NX_GetDeltaTime();

        NX_Begin3D(&camera, NULL, target);
        NX_DrawMesh3D(ground, NULL, NULL);
        NX_DrawModel3D(model, NULL);
        NX_End3D();

        NX_Begin2D(NULL);
        NX_SetShader2D(shader);
        NX_SetTexture2D(NX_GetRenderTexture(target));
        NX_DrawRect2D(0, NX_GetWindowHeight(), NX_GetWindowWidth(), -NX_GetWindowHeight());
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
