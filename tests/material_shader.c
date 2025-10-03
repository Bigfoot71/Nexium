#include <Hyperion/Hyperion.h>
#include "./common.h"
#include "Hyperion/HP_Math.h"
#include "Hyperion/HP_Rand.h"
#include "Hyperion/HP_Render.h"

#define X_INSTANCES 100
#define Z_INSTANCES 100

#define NUM_INSTANCES X_INSTANCES * Z_INSTANCES

static HP_Mat4 iMatrices[NUM_INSTANCES];
static HP_Color iColors[NUM_INSTANCES];
static HP_Vec4 iData[NUM_INSTANCES];

int main(void)
{
    HP_Init("Hyperion - Skybox", 800, 450, HP_FLAG_VSYNC_HINT);
    HP_AddSearchPath(RESOURCES_PATH, false);

    HP_MaterialShader* shader = HP_LoadMaterialShader(
        "shaders/material.vert",
        "shaders/material.frag"
    );

    HP_Mesh* cube = HP_GenMeshCube(HP_VEC3_ONE, HP_VEC3_ONE);
    HP_Material material = HP_GetDefaultMaterial();
    material.emission.energy = 1.0f;
    material.shader = shader;

    HP_InstanceBuffer* instances = HP_CreateInstanceBuffer(
        HP_INSTANCE_DATA_MATRIX | HP_INSTANCE_DATA_COLOR | HP_INSTANCE_DATA_CUSTOM,
        NUM_INSTANCES
    );

    for (int z = 0, i = 0; z < Z_INSTANCES; z++) {
        for (int x = 0; x < X_INSTANCES; x++, i++)
        {
            float px = HP_Remap(x, 0, X_INSTANCES, -100, 100);
            float pz = HP_Remap(z, 0, Z_INSTANCES, -100, 100);

            iMatrices[i] = HP_Mat4Translate(HP_VEC3(px, 0, pz));
            iColors[i] = HP_ColorFromHSV(HP_Wrap(i, 0, 360), 1, 1, 1);
            iData[i] = HP_VEC4(10.0f * HP_RandFloat(NULL), 100.0f * HP_RandFloat(NULL), 0, 0);
        }
    }

    HP_UpdateInstanceBuffer(instances, HP_INSTANCE_DATA_MATRIX, iMatrices, 0, NUM_INSTANCES, false);
    HP_UpdateInstanceBuffer(instances, HP_INSTANCE_DATA_COLOR, iColors, 0, NUM_INSTANCES, false);
    HP_UpdateInstanceBuffer(instances, HP_INSTANCE_DATA_CUSTOM, iData, 0, NUM_INSTANCES, false);

    HP_Camera camera = HP_GetDefaultCamera();
    HP_Environment env = HP_GetDefaultEnvironment();
    env.bloom.mode = HP_BLOOM_MIX;
    env.bloom.strength = 0.1f;
    env.background = HP_BLACK;
    env.ambient = HP_BLACK;

    while (HP_FrameStep())
    {
        CMN_UpdateCamera(&camera, HP_VEC3_ZERO, 2.0f, 1.0f);

        HP_Begin3D(&camera, &env, NULL);
        HP_DrawMeshInstanced3D(cube, instances, NUM_INSTANCES, &material, NULL);
        HP_End3D();
    }

    HP_Quit();

    return 0;
}
