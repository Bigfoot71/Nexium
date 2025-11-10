#include <NX/Nexium.h>
#include "./common.h"

static void BuildWavePlane(NX_DynamicMesh* mesh, float t)
{
    const int resX = 100;
    const int resY = 100;
    const float size = 4.0f;
    const float amplitude = 0.2f;
    const float freqX = 2.0f;
    const float freqY = 2.0f;
    const float speedX = 1.5f;
    const float speedY = 1.0f;

    const NX_DynamicMeshFlags flags = NX_DYNAMIC_MESH_GEN_NORMALS | NX_DYNAMIC_MESH_GEN_TANGENTS;
    NX_BeginDynamicMesh(mesh, NX_PRIMITIVE_TRIANGLES, flags);

    for (int y = 0; y < resY; ++y)
    {
        float v0 = (float)y / resY;
        float v1 = (float)(y + 1) / resY;
        float py0 = (v0 - 0.5f) * size;
        float py1 = (v1 - 0.5f) * size;

        for (int x = 0; x < resX; ++x)
        {
            float u0 = (float)x / resX;
            float u1 = (float)(x + 1) / resX;
            float px0 = (u0 - 0.5f) * size;
            float px1 = (u1 - 0.5f) * size;

            float wave00 = sinf(px0 * freqX + t * speedX) * cosf(py0 * freqY + t * speedY);
            float wave01 = sinf(px0 * freqX + t * speedX) * cosf(py1 * freqY + t * speedY);
            float wave10 = sinf(px1 * freqX + t * speedX) * cosf(py0 * freqY + t * speedY);
            float wave11 = sinf(px1 * freqX + t * speedX) * cosf(py1 * freqY + t * speedY);

            float z00 = wave00 * amplitude;
            float z01 = wave01 * amplitude;
            float z10 = wave10 * amplitude;
            float z11 = wave11 * amplitude;

            NX_Vec3 p00 = NX_VEC3(px0, z00, py0);
            NX_Vec3 p01 = NX_VEC3(px0, z01, py1);
            NX_Vec3 p10 = NX_VEC3(px1, z10, py0);
            NX_Vec3 p11 = NX_VEC3(px1, z11, py1);

            float c00 = 1.0f + z00 * 2.0f;
            float c01 = 1.0f + z01 * 2.0f;
            float c10 = 1.0f + z10 * 2.0f;
            float c11 = 1.0f + z11 * 2.0f;

            NX_Color col00 = NX_COLOR(0.0f, 0.3f * c00, 0.7f * c00, 1.0f);
            NX_Color col01 = NX_COLOR(0.0f, 0.3f * c01, 0.7f * c01, 1.0f);
            NX_Color col10 = NX_COLOR(0.0f, 0.3f * c10, 0.7f * c10, 1.0f);
            NX_Color col11 = NX_COLOR(0.0f, 0.3f * c11, 0.7f * c11, 1.0f);

            NX_SetDynamicMeshColor(mesh, col00); NX_AddDynamicMeshVertex(mesh, p00);
            NX_SetDynamicMeshColor(mesh, col10); NX_AddDynamicMeshVertex(mesh, p11);
            NX_SetDynamicMeshColor(mesh, col11); NX_AddDynamicMeshVertex(mesh, p10);

            NX_SetDynamicMeshColor(mesh, col00); NX_AddDynamicMeshVertex(mesh, p00);
            NX_SetDynamicMeshColor(mesh, col11); NX_AddDynamicMeshVertex(mesh, p01);
            NX_SetDynamicMeshColor(mesh, col01); NX_AddDynamicMeshVertex(mesh, p11);
        }
    }

    NX_EndDynamicMesh(mesh);
}

int main(void)
{
    NX_AppDesc desc = {
        .render3D = { .sampleCount = 4, .resolution = { 800, 450 } },
        .flags = NX_FLAG_VSYNC_HINT
    };

    NX_InitEx("Nexium - Dynamic Mesh", 800, 450, &desc);

    NX_DynamicMesh* dynMesh = NX_CreateDynamicMesh(20000);

    NX_Light* light = NX_CreateLight(NX_LIGHT_DIR);
    NX_SetLightDirection(light, NX_VEC3(-1, -1, -1));
    NX_SetLightActive(light, true);

    NX_Camera cam = NX_GetDefaultCamera();

    while (NX_FrameStep())
    {
        CMN_UpdateCamera(&cam, NX_VEC3(0, 0, 0), 4.0f, 2.0f);
        BuildWavePlane(dynMesh, NX_GetElapsedTime());

        NX_Begin3D(&cam, NULL, NULL);
        NX_DrawDynamicMesh3D(dynMesh, NULL, NULL);
        NX_End3D();
    }

    NX_DestroyDynamicMesh(dynMesh);
    NX_DestroyLight(light);

    NX_Quit();

    return 0;
}
