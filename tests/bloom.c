#include <NX/Nexium.h>
#include "./common.h"

int main(void)
{
    NX_Init("Nexium - Bloom", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_ONE, NX_VEC3_ONE);
    NX_Material material = NX_GetDefaultMaterial();
    material.emission.color = NX_RED;
    material.emission.energy = 10.0f;

    NX_Camera camera = NX_GetDefaultCamera();
    NX_Environment env = NX_GetDefaultEnvironment();
    env.bloom.mode = NX_BLOOM_MIX;
    env.background = NX_BLACK;
    env.ambient = NX_BLACK;

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 2.0f, 1.0f);

        NX_Begin3D(&camera, &env, NULL);
        NX_DrawMesh3D(cube, &material, NULL);
        NX_End3D();
    }

    NX_Quit();

    return 0;
}
