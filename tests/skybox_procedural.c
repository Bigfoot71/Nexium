#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Skybox Procedural", 800, 450, NX_FLAG_VSYNC_HINT);

    NX_Mesh* sphere = NX_GenMeshSphere(0.4f, 32, 16);
    NX_Material material = NX_GetDefaultMaterial();
    material.albedo.color = NX_SILVER;

    NX_Cubemap* skybox = NX_CreateCubemap(1024, NX_PIXEL_FORMAT_RGB16F);
    NX_GenerateSkybox(skybox, &(NX_Skybox) {
        .sunDirection = NX_VEC3(-1, -1, -1),
        .skyColorTop = NX_COLOR(0.38f, 0.45f, 0.55f, 1.0f),
        .skyColorHorizon = NX_COLOR(0.6f, 0.75f, 0.9f, 1.0f),
        .sunColor = NX_COLOR(1.0f, 0.95f, 0.8f, 1.0f),
        .groundColor = NX_COLOR(0.2f, 0.17f, 0.13f, 1.0f),
        .sunSize = 0.02f,
        .haze = 0.1f,
        .energy = 1.0f
    });

    NX_ReflectionProbe* skyprobe = NX_CreateReflectionProbe(skybox);

    NX_Camera camera = NX_GetDefaultCamera();
    NX_Environment env = NX_GetDefaultEnvironment();
    env.tonemap.mode = NX_TONEMAP_ACES;
    env.tonemap.exposure = 4.0f;
    env.tonemap.white = 8.0f;
    env.sky.intensity = 0.2f;
    env.sky.cubemap = skybox;
    env.sky.probe = skyprobe;

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 16.0f, 0.0f);

        NX_Begin3D(&camera, &env, NULL);
        {
            NX_Transform transform = NX_TRANSFORM_IDENTITY;

            for (int x = -5; x <= 5; x++) {
                for (int y = -5; y <= 5; y++) {
                    transform.translation.x = x;
                    transform.translation.y = y;
                    material.orm.roughness = NX_Remap(x, -5, 5, 1, 0);
                    material.orm.metalness = NX_Remap(y, -5, 5, 0, 1);
                    NX_DrawMesh3D(sphere, &material, &transform);
                }
            }
        }
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
