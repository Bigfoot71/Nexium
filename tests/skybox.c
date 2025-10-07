#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Skybox", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Mesh* sphere = NX_GenMeshSphere(0.4f, 32, 16);
    NX_Material material = NX_GetDefaultMaterial();
    material.albedo.color = NX_SILVER;

    NX_Cubemap* skybox = NX_LoadCubemap("cubemaps/panorama.hdr");
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
