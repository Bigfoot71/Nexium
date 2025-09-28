/* PoolModel.cpp -- Storage pool for models and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./PoolModel.hpp"

#include <Hyperion/HP_Render.h>
#include <Hyperion/HP_Image.h>
#include <Hyperion/HP_Math.h>

#include "../../Detail/Helper.hpp"
#include "./PoolTexture.hpp"

#include <assimp/GltfMaterial.h>
#include <assimp/quaternion.h>
#include <assimp/vector3.h>
#include <assimp/types.h>

namespace render {

const aiScene* PoolModel::loadSceneFromMemory(const void* data, uint32_t size, const char* hint)
{
    constexpr uint32_t flags = (
        aiProcess_Triangulate               |
        aiProcess_FlipUVs                   |
        aiProcess_GenNormals                |
        aiProcess_CalcTangentSpace          |
        aiProcess_JoinIdenticalVertices     |
        aiProcess_RemoveRedundantMaterials  |
        aiProcess_GlobalScale
    );

    const aiScene* scene = mImporter.ReadFileFromMemory(data, size, flags, hint);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        HP_INTERNAL_LOG(E, "RENDER: Assimp error; %s", mImporter.GetErrorString());
        mImporter.FreeScene();
        return nullptr;
    }

    return scene;
}

HP_ModelAnimation** PoolModel::loadAnimationsFromScene(const aiScene* scene, int* animCount, int targetFrameRate)
{
    /* --- Check if there are animations --- */

    if (scene->mNumAnimations == 0) {
        HP_INTERNAL_LOG(E, "RENDER: No animations found");
        return nullptr;
    }

    /* --- Allocate animations array --- */

    HP_ModelAnimation** animations = static_cast<HP_ModelAnimation**>(SDL_calloc(scene->mNumAnimations, sizeof(HP_ModelAnimation*)));
    if (animations == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to allocate memory for animations");
        return nullptr;
    }

    /* --- Process each animation --- */

    size_t successCount = 0;
    for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
        animations[successCount] = mPoolAnimation.create();
        const aiAnimation* aiAnim = scene->mAnimations[i];
        if (processAnimation(animations[successCount], scene, aiAnim, targetFrameRate)) {
            successCount++;
        }
        else {
            HP_INTERNAL_LOG(E, "RENDER: Failed to process animation %d", i);
        }
    }

    /* --- Handle results --- */

    if (successCount == 0) {
        HP_INTERNAL_LOG(E, "RENDER: No animations were successfully loaded");
        return nullptr;
    }

    if (successCount < scene->mNumAnimations) {
        HP_INTERNAL_LOG(W, "RENDER: Only %d out of %d animations were successfully loaded", successCount, scene->mNumAnimations);
        HP_ModelAnimation** resizedAnims = static_cast<HP_ModelAnimation**>(SDL_realloc(animations, successCount * sizeof(HP_ModelAnimation*)));
        if (resizedAnims) animations = resizedAnims;
    }

    *animCount = successCount;

    return animations;
}

bool PoolModel::loadModelFromScene(const aiScene* scene, HP_Model* model)
{
    /* --- Process materials --- */

    if (!processMaterials(model, scene)) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to process materials; The model will be invalid");
        return false;
    }

    /* --- Allocate space and process all meshes --- */

    if (!processMeshes(model, scene, scene->mRootNode)) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to process meshes, model will not be animated");
        return false;
    }

    /* --- Process bones and bind poses --- */

    if (!processBones(model, scene)) {
        HP_INTERNAL_LOG(W, "RENDER: Failed to process bones, model will not be animated");
    }

    /* --- Calculate model bounding box --- */

    HP_UpdateModelAABB(model, false);

    return true;
}

} // namespace render
