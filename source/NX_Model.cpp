/* NX_Model.cpp -- API definition for Nexium's model module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <NX/NX_Model.h>

#include "./Importer/AnimationImporter.hpp"
#include "./Importer/MaterialImporter.hpp"
#include "./Importer/SceneImporter.hpp"
#include "./Importer/MeshImporter.hpp"
#include "./Importer/BoneImporter.hpp"
#include "./INX_Utils.hpp"

#include "./INX_GlobalPool.hpp"
#include <NX/NX_Filesystem.h>
#include <NX/NX_Material.h>
#include <NX/NX_Memory.h>
#include <NX/NX_Log.h>

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Model* NX_LoadModel(const char* filePath)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);
    if (fileData == nullptr || fileSize == 0) {
        NX_LOG(E, "RENDER: Failed to load model data: %s", filePath);
        return nullptr;
    }

    NX_Model* model = NX_LoadModelFromData(fileData, fileSize, INX_GetFileExt(filePath));
    NX_Free(fileData);

    return model;
}

NX_Model* NX_LoadModelFromData(const void* data, size_t size, const char* hint)
{
    import::SceneImporter importer(data, size, hint);
    if (!importer.isValid()) {
        return nullptr;
    }

    NX_Model* model = INX_Pool.Create<NX_Model>();
    if (model == nullptr) {
        NX_LOG(E, "RENDER: Failed to load model; Object pool issue");
        return nullptr;
    }

    if (!import::MeshImporter(importer).loadMeshes(model)) {
        NX_DestroyModel(model);
        return nullptr;
    }

    if (!import::MaterialImporter(importer).loadMaterials(model)) {
        NX_DestroyModel(model);
        return nullptr;
    }

    if (!import::BoneImporter(importer).processBones(model)) {
        NX_DestroyModel(model);
        return nullptr;
    }

    return model;
}

void NX_DestroyModel(NX_Model* model)
{
    if (model == nullptr) return;

    for (int i = 0; i < model->meshCount; i++) {
        if (model->meshes[i] != nullptr) {
            NX_DestroyMesh(model->meshes[i]);
        }
    }

    for (int i = 0; i < model->materialCount; i++) {
        NX_DestroyMaterialResources(&model->materials[i]);
    }

    NX_Free(model->meshes);
    NX_Free(model->meshMaterials);
    NX_Free(model->materials);
    NX_Free(model->bones);
    NX_Free(model->boneOffsets);

    INX_Pool.Destroy(model);
}

void NX_UpdateModelAABB(NX_Model* model, bool updateMeshAABBs)
{
    if (!model || !model->meshes) {
        return;
    }

    NX_Vec3 min = { +FLT_MAX, +FLT_MAX, +FLT_MAX };
    NX_Vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (uint32_t i = 0; i < model->meshCount; i++) {
        NX_Mesh* mesh = model->meshes[i];
        if (updateMeshAABBs) {
            NX_UpdateMeshAABB(mesh);
        }
        min = NX_Vec3Min(min, mesh->aabb.min);
        max = NX_Vec3Max(max, mesh->aabb.max);
    }

    model->aabb.min = min;
    model->aabb.max = max;
}

void NX_ScaleModelAABB(NX_Model* model, float scale, bool scaleMeshAABBs)
{
    if (scaleMeshAABBs) {
        for (int i = 0; i < model->meshCount; i++) {
            model->meshes[i]->aabb.min *= scale;
            model->meshes[i]->aabb.max *= scale;
        }
    }

    model->aabb.min *= scale;
    model->aabb.max *= scale;
}

NX_Animation** NX_LoadAnimations(const char* filePath, int* animCount, int targetFrameRate)
{
    size_t fileSize = 0;
    void* fileData = NX_LoadFile(filePath, &fileSize);

    NX_Animation** animations = NX_LoadAnimationFromData(
        fileData, fileSize, INX_GetFileExt(filePath),
        animCount, targetFrameRate
    );

    NX_Free(fileData);

    return animations;
}

NX_Animation** NX_LoadAnimationFromData(const void* data, unsigned int size, const char* hint, int* animCount, int targetFrameRate)
{
    // TODO: Review how animations are loaded. I was thinking of creating a separate 'PoolAnimation'
    //       and introducing a new type 'NX_AnimationLibrary' instead of returning arrays of pointers.

    import::SceneImporter importer(data, size, hint);
    if (!importer.isValid()) {
        return nullptr;
    }

    return import::AnimationImporter(importer).loadAnimations(animCount, targetFrameRate);
}

void NX_DestroyAnimations(NX_Animation** animations, int animCount)
{
    if (animations != nullptr) {
        for (int i = 0; i < animCount; i++) {
            INX_Pool.Destroy(animations[i]);
        }
        NX_Free(animations);
    }
}

NX_Animation* NX_GetAnimation(NX_Animation** animations, int animCount, const char* name)
{
    for (int i = 0; i < animCount; i++) {
        if (SDL_strcmp(animations[i]->name, name) == 0) {
            return animations[i];
        }
    }
    return nullptr;
}
