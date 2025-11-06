/* instanced_material_shader.c -- Instanced rendering test using a custom material shader
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/Nexium.h>
#include "./common.h"

#define X_INSTANCES 100
#define Z_INSTANCES 100
#define NUM_INSTANCES (X_INSTANCES * Z_INSTANCES)

/* --- Instance data --- */

static NX_Vec3  iPositions[NUM_INSTANCES];
static NX_Color iColors[NUM_INSTANCES];
static NX_Vec4  iData[NUM_INSTANCES];

int main(void)
{
    /* --- Initialize engine and paths --- */

    NX_Init("Nexium - Instanced Material Shader", 800, 450, NX_FLAG_VSYNC_HINT);
    NX_AddSearchPath(RESOURCES_PATH, false);

    /* --- Load shader and setup material --- */

    NX_Shader3D* shader = NX_LoadShader3D(
        "shaders/instanced_material.vert",
        "shaders/instanced_material.frag"
    );

    NX_Mesh* cube = NX_GenMeshCube(NX_VEC3_ONE, NX_IVEC3_ONE);
    NX_Material material = NX_GetDefaultMaterial();
    material.emission.energy = 1.0f;
    material.shader = shader;

    /* --- Create and populate instance buffer --- */

    NX_InstanceBuffer* instances = NX_CreateInstanceBuffer(
        NX_INSTANCE_POSITION | NX_INSTANCE_COLOR | NX_INSTANCE_CUSTOM,
        NUM_INSTANCES
    );

    for (int z = 0, i = 0; z < Z_INSTANCES; z++)
     for (int x = 0; x < X_INSTANCES; x++, i++) {
        float px = NX_Remap(x, 0, X_INSTANCES, -100, 100);
        float pz = NX_Remap(z, 0, Z_INSTANCES, -100, 100);

        iPositions[i] = NX_VEC3(px, 0, pz);
        iColors[i]    = NX_ColorFromHSV(NX_Wrap(i, 0, 360), 1, 1, 1);
        iData[i]      = NX_VEC4(
            10.0f  * NX_RandFloat(NULL),
            100.0f * NX_RandFloat(NULL),
            0, 0
        );
    }

    NX_UpdateInstanceBuffer(instances, NX_INSTANCE_POSITION, 0, NUM_INSTANCES, iPositions);
    NX_UpdateInstanceBuffer(instances, NX_INSTANCE_COLOR,   0, NUM_INSTANCES, iColors);
    NX_UpdateInstanceBuffer(instances, NX_INSTANCE_CUSTOM,  0, NUM_INSTANCES, iData);

    /* --- Setup camera and environment --- */

    NX_Camera camera    = NX_GetDefaultCamera();
    NX_Environment env  = NX_GetDefaultEnvironment();
    env.bloom.mode      = NX_BLOOM_MIX;
    env.bloom.strength  = 0.1f;
    env.background      = NX_BLACK;
    env.ambient         = NX_BLACK;

    /* --- Main loop --- */

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&camera, NX_VEC3_ZERO, 2.0f, 1.0f);

        NX_Begin3D(&camera, &env, NULL);
        NX_DrawMeshInstanced3D(cube, instances, NUM_INSTANCES, &material, NULL);
        NX_End3D();
    }

    /* --- Cleanup --- */

    NX_DestroyInstanceBuffer(instances);
    NX_DestroyShader3D(shader);
    NX_DestroyMesh(cube);

    NX_Quit();

    return 0;
}
