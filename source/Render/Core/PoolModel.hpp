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
#include "./PoolTexture.hpp"
#include "./PoolMesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>

namespace render {

/* === Declaration === */

class PoolModel {
public:
    PoolModel(PoolTexture& poolTexture, PoolMesh& poolMesh);
    void setImportScale(float scale);

    NX_Model* loadModel(const void* fileData, size_t fileSize, const char* hint);
    void destroyModel(NX_Model* model);

    NX_ModelAnimation** loadAnimations(const void* fileData, size_t fileSize, const char* hint, int* animCount, int targetFrameRate);
    void destroyAnimations(NX_ModelAnimation** animations, int count);

private:
    const aiScene* loadSceneFromMemory(const void* data, uint32_t size, const char* hint);
    NX_ModelAnimation** loadAnimationsFromScene(const aiScene* scene, int* animCount, int targetFrameRate);
    bool loadModelFromScene(const aiScene* scene, NX_Model* model);

private:
    /** PoolModelAnimation.cpp */
    static bool processAnimation(NX_ModelAnimation* animation, const struct aiScene* scene, const struct aiAnimation* aiAnim, int targetFrameRate);

    /** PoolModelMesh.cpp */
    template <bool HasBones>
    NX_Mesh* processMesh(const aiMesh* mesh, const NX_Mat4& transform);
    bool processMeshesRecursive(NX_Model* model, const aiScene* scene, const aiNode* node, const NX_Mat4& parentFinalTransform);
    bool processMeshes(NX_Model* model, const aiScene* scene, const aiNode* node);

    /** PoolModelMaterial.cpp */
    NX_Texture* loadTexture(const aiScene* scene, const aiMaterial* aiMat, aiTextureType type, uint32_t index, bool asData);
    NX_Texture* loadTextureORM(const aiScene* scene, const aiMaterial* aiMat, bool* hasOcclusion, bool* hasRoughness, bool* hasMetalness);
    bool processMaterials(NX_Model* model, const aiScene* scene);

    /** PoolModelBones.cpp */
    bool processBones(NX_Model* model, const aiScene* scene);

private:
    util::ObjectPool<NX_ModelAnimation, 256> mPoolAnimation;
    util::ObjectPool<NX_Model, 128> mPoolModel;
    Assimp::Importer mImporter;
    PoolTexture& mPoolTexture;
    PoolMesh& mPoolMesh;
};

/* === Public Implementation === */

inline PoolModel::PoolModel(PoolTexture& poolTexture, PoolMesh& poolMesh)
    : mPoolTexture(poolTexture)
    , mPoolMesh(poolMesh)
{
    mImporter.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
}

inline void PoolModel::setImportScale(float scale)
{
    mImporter.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale);
}

inline NX_Model* PoolModel::loadModel(const void* fileData, size_t fileSize, const char* hint)
{
    const aiScene* scene = loadSceneFromMemory(fileData, fileSize, hint);
    if (scene == nullptr) {
        return nullptr;
    }

    NX_Model* model = mPoolModel.create();
    if (model == nullptr) {
        return nullptr;
    }

    if (!loadModelFromScene(scene, model)) {
        destroyModel(model);
        model = nullptr;
    }

    mImporter.FreeScene();

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
        mPoolTexture.destroyTexture(mat.albedo.texture);
        mPoolTexture.destroyTexture(mat.normal.texture);
        mPoolTexture.destroyTexture(mat.emission.texture);
        mPoolTexture.destroyTexture(mat.orm.texture);
    }

    SDL_free(model->meshes);
    SDL_free(model->meshMaterials);
    SDL_free(model->materials);
    SDL_free(model->bones);
    SDL_free(model->boneOffsets);

    mPoolModel.destroy(model);
}

inline NX_ModelAnimation** PoolModel::loadAnimations(const void* fileData, size_t fileSize, const char* hint, int* animCount, int targetFrameRate)
{
    const aiScene* scene = loadSceneFromMemory(fileData, fileSize, hint);
    if (scene == nullptr) {
        return nullptr;
    }

    NX_ModelAnimation** animations = loadAnimationsFromScene(scene, animCount, targetFrameRate);

    mImporter.FreeScene();

    return animations;
}

inline void PoolModel::destroyAnimations(NX_ModelAnimation** animations, int count)
{
    if (animations != nullptr) {
        for (int i = 0; i < count; i++) {
            mPoolAnimation.destroy(animations[i]);
        }
        SDL_free(animations);
    }
}

} // namespace render

#endif // NX_RENDER_POOL_MODEL_HPP
