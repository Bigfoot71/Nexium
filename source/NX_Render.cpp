/* NX_Render.cpp -- API definition for Nexium's render module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Filesystem.h>
#include <NX/NX_Codepoint.h>
#include <NX/NX_Render2D.h>
#include <NX/NX_Texture.h>
#include <NX/NX_Render.h>
#include <NX/NX_Image.h>
#include <NX/NX_Font.h>
#include <NX/NX_Math.h>
#include <NX/NX_Log.h>

#include "./NX_RenderTexture.hpp"
#include "./NX_Texture.hpp"
#include "./NX_Font.hpp"

#include "./Render/NX_RenderState.hpp"
#include "./NX_InstanceBuffer.hpp"
#include "./INX_GlobalAssets.hpp"
#include "./Detail/Helper.hpp"

/* === Draw3D - Public API === */

void NX_Begin3D(const NX_Camera* camera, const NX_Environment* env, const NX_RenderTexture* target)
{
    gRender->scene.begin(
        camera ? *camera : NX_GetDefaultCamera(),
        env ? *env : NX_GetDefaultEnvironment(),
        target
    );
}

void NX_End3D()
{
    gRender->scene.end();
}

void NX_DrawMesh3D(const NX_Mesh* mesh, const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        mesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawMeshInstanced3D(const NX_Mesh* mesh, const NX_InstanceBuffer* instances, int instanceCount,
                            const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        mesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMesh3D(const NX_DynamicMesh* dynMesh, const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        dynMesh, nullptr, 0,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawDynamicMeshInstanced3D(const NX_DynamicMesh* dynMesh, const NX_InstanceBuffer* instances, int instanceCount,
                                   const NX_Material* material, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        dynMesh, instances, instanceCount,
        material ? *material : NX_GetDefaultMaterial(),
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModel3D(const NX_Model* model, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        *model, nullptr, 0,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}

void NX_DrawModelInstanced3D(const NX_Model* model, const NX_InstanceBuffer* instances, int instanceCount, const NX_Transform* transform)
{
    gRender->scene.drawCalls().push(
        *model, instances, instanceCount,
        transform ? *transform : NX_TRANSFORM_IDENTITY
    );
}
