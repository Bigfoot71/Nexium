/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef HP_RENDER_POOL_MODEL_HPP
#define HP_RENDER_POOL_MODEL_HPP

#include <Hyperion/HP_Render.h>

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

    HP_Model* loadModel(const void* fileData, size_t fileSize, const char* hint);
    void destroyModel(HP_Model* model);

    HP_ModelAnimation** loadAnimations(const void* fileData, size_t fileSize, const char* hint, int* animCount, int targetFrameRate);
    void destroyAnimations(HP_ModelAnimation** animations, int count);

private:
    const aiScene* loadSceneFromMemory(const void* data, uint32_t size, const char* hint);
    HP_ModelAnimation** loadAnimationsFromScene(const aiScene* scene, int* animCount, int targetFrameRate);
    bool loadModelFromScene(const aiScene* scene, HP_Model* model);

private:
    /** PoolModelAnimation.cpp */
    static bool processAnimation(HP_ModelAnimation* animation, const struct aiScene* scene, const struct aiAnimation* aiAnim, int targetFrameRate);

    /** PoolModelMesh.cpp */
    template <bool HasBones>
    HP_Mesh* processMesh(const aiMesh* mesh, const HP_Mat4& transform);
    bool processMeshesRecursive(HP_Model* model, const aiScene* scene, const aiNode* node, const HP_Mat4& parentFinalTransform);
    bool processMeshes(HP_Model* model, const aiScene* scene, const aiNode* node);

    /** PoolModelMaterial.cpp */
    HP_Texture* loadTexture(const aiScene* scene, const aiMaterial* aiMat, aiTextureType type, uint32_t index);
    HP_Texture* loadTextureORM(const aiScene* scene, const aiMaterial* aiMat, bool* hasOcclusion, bool* hasRoughness, bool* hasMetalness);
    bool processMaterials(HP_Model* model, const aiScene* scene);

    /** PoolModelBones.cpp */
    bool processBones(HP_Model* model, const aiScene* scene);

private:
    util::ObjectPool<HP_ModelAnimation, 256> mPoolAnimation;
    util::ObjectPool<HP_Model, 128> mPoolModel;
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

inline HP_Model* PoolModel::loadModel(const void* fileData, size_t fileSize, const char* hint)
{
    const aiScene* scene = loadSceneFromMemory(fileData, fileSize, hint);
    if (scene == nullptr) {
        return nullptr;
    }

    HP_Model* model = mPoolModel.create();
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

inline void PoolModel::destroyModel(HP_Model* model)
{
    if (model == nullptr) return;

    // Libérer les meshes via PoolMesh
    for (int i = 0; i < model->meshCount; i++) {
        if (model->meshes[i]) {
            mPoolMesh.destroy(model->meshes[i]);
        }
    }

    // Libérer les textures via PoolTexture
    for (int i = 0; i < model->materialCount; i++) {
        HP_Material& mat = model->materials[i];
        if (mat.albedo.texture) mPoolTexture.destroy(mat.albedo.texture);
        if (mat.normal.texture) mPoolTexture.destroy(mat.normal.texture);
        if (mat.emission.texture) mPoolTexture.destroy(mat.emission.texture);
        if (mat.orm.texture) mPoolTexture.destroy(mat.orm.texture);
    }

    // Libérer les arrays
    SDL_free(model->meshes);
    SDL_free(model->meshMaterials);
    SDL_free(model->materials);
    SDL_free(model->bones);
    SDL_free(model->boneOffsets);

    // Libère l'espace dans la pool
    mPoolModel.destroy(model);
}

inline HP_ModelAnimation** PoolModel::loadAnimations(const void* fileData, size_t fileSize, const char* hint, int* animCount, int targetFrameRate)
{
    const aiScene* scene = loadSceneFromMemory(fileData, fileSize, hint);
    if (scene == nullptr) {
        return nullptr;
    }

    HP_ModelAnimation** animations = loadAnimationsFromScene(scene, animCount, targetFrameRate);

    mImporter.FreeScene();

    return animations;
}

inline void PoolModel::destroyAnimations(HP_ModelAnimation** animations, int count)
{
    if (animations != nullptr) {
        for (int i = 0; i < count; i++) {
            mPoolAnimation.destroy(animations[i]);
        }
        SDL_free(animations);
    }
}

} // namespace render

#endif // HP_RENDER_POOL_MODEL_HPP
