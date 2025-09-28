/* PoolModelBone.cpp -- Contains the implementation for model bone loading
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "../PoolModel.hpp"
#include "./AssimpHelper.hpp"

/* === Helpers === */

namespace {

int findBoneIndex(const char* name, HP_BoneInfo* bones, int count)
{
    for (int i = 0; i < count; i++) {
        if (SDL_strcmp(name, bones[i].name) == 0) return i;
    }
    return -1;
}

aiMatrix4x4 getGlobalNodeTransform(const aiNode* node, const aiNode* rootNode)
{
    aiMatrix4x4 globalTransform;
    const aiNode* current = node;

    while (current && current != rootNode->mParent) {
        globalTransform = current->mTransformation * globalTransform;
        current = current->mParent;
    }

    return globalTransform;
}

const aiNode* findNodeByName(const aiNode* node, const char* name)
{
    if (SDL_strcmp(node->mName.data, name) == 0) return node;

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        const aiNode* found = findNodeByName(node->mChildren[i], name);
        if (found) return found;
    }

    return nullptr;
}

void buildHierarchyRecursive(const aiNode* node, HP_BoneInfo* bones, int boneCount, int parentIndex)
{
    int currentIndex = findBoneIndex(node->mName.data, bones, boneCount);

    if (currentIndex != -1) {
        bones[currentIndex].parent = parentIndex;
        parentIndex = currentIndex;
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        buildHierarchyRecursive(node->mChildren[i], bones, boneCount, parentIndex);
    }
}

} // namespace

/* === Implementation === */

namespace render {

bool PoolModel::processBones(HP_Model* model, const aiScene* scene)
{
    SDL_assert(scene != nullptr);

    /* --- Count maximum possible bones across all meshes --- */

    int maxPossibleBones = 0;
    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
        maxPossibleBones += scene->mMeshes[i]->mNumBones;
    }

    /* --- Early exit if no bones found --- */

    if (maxPossibleBones == 0) {
        model->boneCount = 0;
        model->bones = nullptr;
        model->boneOffsets = nullptr;
        model->boneBindPose = nullptr;
        return true;
    }

    /* --- Allocate bone arrays --- */

    model->boneOffsets = static_cast<HP_Mat4*>(SDL_malloc(maxPossibleBones * sizeof(HP_Mat4)));
    model->boneBindPose = static_cast<HP_Mat4*>(SDL_malloc(maxPossibleBones * sizeof(HP_Mat4)));
    model->bones = static_cast<HP_BoneInfo*>(SDL_malloc(maxPossibleBones * sizeof(HP_BoneInfo)));

    if (!model->boneOffsets || !model->boneBindPose || !model->bones) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to allocate memory for model bones");
        SDL_free(model->boneBindPose);
        SDL_free(model->boneOffsets);
        SDL_free(model->bones);
        model->boneBindPose = nullptr;
        model->boneOffsets = nullptr;
        model->bones = nullptr;
        model->boneCount = 0;
        return false;
    }

    /* --- Collect unique bones with offset and bind pose matrices --- */

    int uniqueBoneCount = 0;
    for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
        const aiMesh* mesh = scene->mMeshes[m];
        for (uint32_t b = 0; b < mesh->mNumBones; b++) {
            const aiBone* bone = mesh->mBones[b];
            if (findBoneIndex(bone->mName.data, model->bones, uniqueBoneCount) == -1)
            {
                SDL_strlcpy(model->bones[uniqueBoneCount].name, bone->mName.data, sizeof(model->bones[uniqueBoneCount].name));
                model->bones[uniqueBoneCount].parent = -1;

                model->boneOffsets[uniqueBoneCount] = assimp_cast<HP_Mat4>(bone->mOffsetMatrix);

                const aiNode* boneNode = findNodeByName(scene->mRootNode, bone->mName.data);
                if (boneNode) {
                    aiMatrix4x4 globalTransform = getGlobalNodeTransform(boneNode, scene->mRootNode);
                    model->boneBindPose[uniqueBoneCount] = assimp_cast<HP_Mat4>(globalTransform);
                } else {
                    aiMatrix4x4 bindPoseMatrix = bone->mOffsetMatrix;
                    model->boneBindPose[uniqueBoneCount] = assimp_cast<HP_Mat4>(bindPoseMatrix.Inverse());
                }

                uniqueBoneCount++;
            }
        }
    }

    model->boneCount = uniqueBoneCount;

    /* --- Shrink arrays to actual bone count --- */

    if (uniqueBoneCount < maxPossibleBones) {
        void* boneOffsets = SDL_realloc(model->boneOffsets, uniqueBoneCount * sizeof(HP_Mat4));
        void* boneBindPose = SDL_realloc(model->boneBindPose, uniqueBoneCount * sizeof(HP_Mat4));
        void* bones = SDL_realloc(model->bones, uniqueBoneCount * sizeof(HP_BoneInfo));
        if (boneOffsets) model->boneOffsets = static_cast<HP_Mat4*>(boneOffsets);
        if (boneBindPose) model->boneBindPose = static_cast<HP_Mat4*>(boneBindPose);
        if (bones) model->bones = static_cast<HP_BoneInfo*>(bones);
    }

    /* --- Build bone hierarchy from scene graph --- */

    buildHierarchyRecursive(scene->mRootNode, model->bones, model->boneCount, -1);

    return true;
}

} // namespace render
