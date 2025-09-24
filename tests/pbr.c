#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_AppDesc desc = {
        .render3D.resolution = { 800, 450 },
        .render3D.sampleCount = 4,
        .targetFPS = 60
    };

    HP_InitEx("Hyperion - PBR", 800, 450, &desc);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_SetDefaultTextureFilter(HP_TEXTURE_FILTER_TRILINEAR);
    HP_SetDefaultTextureAnisotropy(4.0f);

    HP_Model* model = HP_LoadModel("models/DamagedHelmet.glb");

    HP_Cubemap* skybox = HP_LoadCubemap("cubemaps/panorama.hdr");
    HP_ReflectionProbe* skyprobe = HP_CreateReflectionProbe(skybox);

    HP_Light* light = HP_CreateLight(HP_LIGHT_DIR);
    HP_SetLightDirection(light, HP_VEC3(-1, -1, -1));
    HP_SetShadowActive(light, true);
    HP_SetLightActive(light, true);

    HP_Camera camera = HP_GetDefaultCamera();

    HP_Environment envs[2] = { 0 };

    envs[0] = HP_GetDefaultEnvironment();
    envs[0].ambient = HP_COLOR_1(0.1f);
    envs[0].background = HP_BLACK;

    envs[1] = HP_GetDefaultEnvironment();
    envs[1].tonemap.mode = HP_TONEMAP_ACES;
    envs[1].tonemap.exposure = 2.0f;
    envs[1].tonemap.white = 8.0f;
    envs[1].sky.intensity = 0.2f;
    envs[1].sky.cubemap = skybox;
    envs[1].sky.probe = skyprobe;

    int env = 0;

    while (HP_FrameStep()) {
        if (HP_IsKeyJustPressed(HP_KEY_SPACE)) env = (env + 1) % HP_ARRAY_SIZE(envs);
        CMN_UpdateCamera(&camera, HP_VEC3_ZERO, 2.5f, 1.0f);
        HP_Begin3D(&camera, &envs[env]);
        HP_DrawModel3D(model, NULL);
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
