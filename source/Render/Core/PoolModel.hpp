/* PoolModel.hpp -- Storage pool for models and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_POOL_MODEL_HPP
#define NX_RENDER_POOL_MODEL_HPP

#include <NX/NX_Render.h>

#include "../../Detail/Util/ObjectPool.hpp"
#include "./Importer/AnimationImporter.hpp"
#include "./Importer/MaterialImporter.hpp"
#include "./Importer/SceneImporter.hpp"
#include "./Importer/MeshImporter.hpp"
#include "./Importer/BoneImporter.hpp"
#include "./PoolMesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>

namespace render {

/* === Declaration === */

class PoolModel {
public:
    PoolModel(PoolMesh& poolMesh);

    NX_Model* loadModel(const void* fileData, size_t fileSize, const char* hint);
    void destroyModel(NX_Model* model);

    NX_ModelAnimation** loadAnimations(const void* fileData, size_t fileSize, const char* hint, int* animCount, int targetFrameRate);
    void destroyAnimations(NX_ModelAnimation** animations, int count);

private:
    util::ObjectPool<NX_ModelAnimation, 256> mPoolAnimation;
    util::ObjectPool<NX_Model, 128> mPoolModel;
    PoolMesh& mPoolMesh;
};

/* === Public Implementation === */

inline PoolModel::PoolModel(PoolMesh& poolMesh)
    : mPoolMesh(poolMesh)
{ }

inline NX_Model* PoolModel::loadModel(const void* fileData, size_t fileSize, const char* hint)
{
    SceneImporter importer(fileData, fileSize, hint);
    if (!importer.isValid()) {
        return nullptr;
    }

    NX_Model* model = mPoolModel.create();
    if (model == nullptr) {
        NX_LOG(E, "RENDER: Failed to load model; Object pool issue");
        return nullptr;
    }

    if (!MeshImporter(importer, mPoolMesh).loadMeshes(model)) {
        destroyModel(model);
        return nullptr;
    }

    if (!MaterialImporter(importer).loadMaterials(model)) {
        destroyModel(model);
        return nullptr;
    }

    if (!BoneImporter(importer).processBones(model)) {
        destroyModel(model);
        return nullptr;
    }

    return model;
}

inline void PoolModel::destroyModel(NX_Model* model)
{
    if (model == nullptr) return;

    for (int i = 0; i < model->meshCount; i++) {
        if (model->meshes[i]) {
            mPoolMesh.destroyMesh(model->meshes[i]);
        }
    }

    for (int i = 0; i < model->materialCount; i++) {
        NX_Material& mat = model->materials[i];
        NX_DestroyTexture(mat.albedo.texture);
        NX_DestroyTexture(mat.normal.texture);
        NX_DestroyTexture(mat.emission.texture);
        NX_DestroyTexture(mat.orm.texture);
    }

    NX_Free(model->meshes);
    NX_Free(model->meshMaterials);
    NX_Free(model->materials);
    NX_Free(model->bones);
    NX_Free(model->boneOffsets);

    mPoolModel.destroy(model);
}

inline NX_ModelAnimation** PoolModel::loadAnimations(const void* fileData, size_t fileSize, const char* hint, int* animCount, int targetFrameRate)
{
    // TODO: Review how animations are loaded. I was thinking of creating a separate 'PoolAnimation'
    //       and introducing a new type 'NX_AnimationLibrary' instead of returning arrays of pointers.

    SceneImporter importer(fileData, fileSize, hint);
    if (!importer.isValid()) {
        return nullptr;
    }

    return AnimationImporter(importer, mPoolAnimation).loadAnimations(animCount, targetFrameRate);
}

inline void PoolModel::destroyAnimations(NX_ModelAnimation** animations, int count)
{
    if (animations != nullptr) {
        for (int i = 0; i < count; i++) {
            mPoolAnimation.destroy(animations[i]);
        }
        NX_Free(animations);
    }
}

} // namespace render

#endif // NX_RENDER_POOL_MODEL_HPP
