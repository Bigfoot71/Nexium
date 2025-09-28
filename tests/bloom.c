#include <Hyperion/Hyperion.h>
#include "./common.h"

int main(void)
{
    HP_Init("Hyperion - Skybox", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_ONE, HP_VEC3_ONE);
    HP_Material material = HP_GetDefaultMaterial();
    material.emission.color = HP_RED;
    material.emission.energy = 10.0f;

    HP_Camera camera = HP_GetDefaultCamera();
    HP_Environment env = HP_GetDefaultEnvironment();
    env.bloom.mode = HP_BLOOM_MIX;
    env.background = HP_BLACK;
    env.ambient = HP_BLACK;

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3_ZERO, 2.0f, 1.0f);

        HP_Begin3D(&camera, &env, NULL);
        HP_DrawMesh3D(cube, &material, NULL);
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
