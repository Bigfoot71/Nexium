#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_Init("Hyperion - Skybox Procedural", 800, 450, HP_FLAG_VSYNC_HINT);

    HP_Mesh* sphere = HP_GenMeshSphere(0.4f, 32, 16);
    HP_Material material = HP_GetDefaultMaterial();
    material.albedo.color = HP_SILVER;

    HP_Cubemap* skybox = HP_CreateCubemap(1024, HP_PIXEL_FORMAT_RGB16F);
    HP_GenerateSkybox(skybox, &(HP_Skybox) {
        .sunDirection = HP_VEC3(-1, -1, -1),
        .skyColorTop = HP_COLOR(0.38f, 0.45f, 0.55f, 1.0f),
        .skyColorHorizon = HP_COLOR(0.6f, 0.75f, 0.9f, 1.0f),
        .sunColor = HP_COLOR(1.0f, 0.95f, 0.8f, 1.0f),
        .groundColor = HP_COLOR(0.2f, 0.17f, 0.13f, 1.0f),
        .sunSize = 0.02f,
        .haze = 0.1f,
        .energy = 1.0f
    });

    HP_ReflectionProbe* skyprobe = HP_CreateReflectionProbe(skybox);

    HP_Camera camera = HP_GetDefaultCamera();
    HP_Environment env = HP_GetDefaultEnvironment();
    env.tonemap.mode = HP_TONEMAP_ACES;
    env.tonemap.exposure = 4.0f;
    env.tonemap.white = 8.0f;
    env.sky.intensity = 0.2f;
    env.sky.cubemap = skybox;
    env.sky.probe = skyprobe;

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3_ZERO, 16.0f, 0.0f);

        HP_Begin3D(&camera, &env, NULL);
        {
            HP_Transform transform = HP_TRANSFORM_IDENTITY;

            for (int x = -5; x <= 5; x++) {
                for (int y = -5; y <= 5; y++) {
                    transform.translation.x = x;
                    transform.translation.y = y;
                    material.orm.roughness = HP_Remap(x, -5, 5, 1, 0);
                    material.orm.metalness = HP_Remap(y, -5, 5, 0, 1);
                    HP_DrawMesh3D(sphere, &material, &transform);
                }
            }
        }
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
