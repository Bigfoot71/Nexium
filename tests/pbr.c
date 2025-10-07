#include <NX/Nexium.h>
#include "./common.h"
#include "NX/NX_Render.h"

int main(void)
{
    NX_AppDesc desc = {
        .render3D.resolution = { 800, 450 },
        .render3D.sampleCount = 4,
        .targetFPS = 60
    };

    NX_InitEx("Nexium - PBR", 800, 450, &desc);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_SetDefaultTextureFilter(NX_TEXTURE_FILTER_TRILINEAR);
    NX_SetDefaultTextureAnisotropy(4.0f);

    NX_Model* model = NX_LoadModel("models/DamagedHelmet.glb");

    NX_Cubemap* skybox = NX_LoadCubemap("cubemaps/panorama.hdr");
    NX_ReflectionProbe* skyprobe = NX_CreateReflectionProbe(skybox);

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetShadowActive(light, true);
    NX_SetLightActive(light, true);

    NX_Camera camera = NX_GetDefaultCamera();

    NX_Environment envs[2] = { 0 };

    envs[0] = NX_GetDefaultEnvironment();
    envs[0].ambient = NX_COLOR_1(0.1f);
    envs[0].background = NX_BLACK;

    envs[1] = NX_GetDefaultEnvironment();
    envs[1].bloom.mode = NX_BLOOM_MIX;
    envs[1].bloom.strength = 0.08f;
    envs[1].tonemap.mode = NX_TONEMAP_ACES;
    envs[1].tonemap.exposure = 2.0f;
    envs[1].tonemap.white = 8.0f;
    envs[1].sky.intensity = 0.2f;
    envs[1].sky.cubemap = skybox;
    envs[1].sky.probe = skyprobe;

    int env = 0;

    while (NX_FrameStep()) {
        if (NX_IsKeyJustPressed(NX_KEY_SPACE)) env = (env + 1) % NX_ARRAY_SIZE(envs);
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 2.5f, 1.0f);
        NX_Begin3D(&camera, &envs[env], NULL);
        NX_DrawModel3D(model, NULL);
        NX_End3D();
        NX_Begin2D(NULL);
        NX_DrawText2D("Press SPACE to change environment", NX_VEC2_1(10), 32, NX_VEC2_ONE);
        NX_End2D();
    }

    NX_Quit();

    return 0;
}
