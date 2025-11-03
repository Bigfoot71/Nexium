#ifndef NX_IMPORT_BONE_IMPORTER_HPP
#define NX_IMPORT_BONE_IMPORTER_HPP

#include <NX/NX_Render.h>

#include "./SceneImporter.hpp"
#include "./AssimpHelper.hpp"

#include <SDL3/SDL_assert.h>
#include <assimp/mesh.h>
#include <float.h>

namespace import {

/* === Declaration === */

class BoneImporter {
public:
    /** Constructors */
    BoneImporter(const SceneImporter& importer);

    /** Loads the bones and stores them in the specified model */
    bool processBones(NX_Model* model);

private:
    static int findBoneIndex(const char* name, NX_BoneInfo* bones, int count);
    static aiMatrix4x4 getGlobalNodeTransform(const aiNode* node, const aiNode* rootNode);
    static const aiNode* findNodeByName(const aiNode* node, const char* name);
    static void buildHierarchyRecursive(const aiNode* node, NX_BoneInfo* bones, int boneCount, int parentIndex);

private:
    const SceneImporter& mImporter;
};

/* === Public Implementation === */

inline BoneImporter::BoneImporter(const SceneImporter& importer)
    : mImporter(importer)
{
    SDL_assert(importer.isValid());
}

inline bool BoneImporter::processBones(NX_Model* model)
{
    /* --- Count maximum possible bones across all meshes --- */

    int maxPossibleBones = 0;
    for (uint32_t i = 0; i < mImporter.meshCount(); i++) {
        maxPossibleBones += mImporter.mesh(i)->mNumBones;
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

    model->boneOffsets = static_cast<NX_Mat4*>(SDL_malloc(maxPossibleBones * sizeof(NX_Mat4)));
    model->boneBindPose = static_cast<NX_Mat4*>(SDL_malloc(maxPossibleBones * sizeof(NX_Mat4)));
    model->bones = static_cast<NX_BoneInfo*>(SDL_malloc(maxPossibleBones * sizeof(NX_BoneInfo)));

    if (!model->boneOffsets || !model->boneBindPose || !model->bones) {
        NX_LOG(E, "RENDER: Failed to allocate memory for model bones");
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
    for (uint32_t m = 0; m < mImporter.meshCount(); m++) {
        const aiMesh* mesh = mImporter.mesh(m);
        for (uint32_t b = 0; b < mesh->mNumBones; b++) {
            const aiBone* bone = mesh->mBones[b];
            if (findBoneIndex(bone->mName.data, model->bones, uniqueBoneCount) == -1)
            {
                SDL_strlcpy(model->bones[uniqueBoneCount].name, bone->mName.data, sizeof(model->bones[uniqueBoneCount].name));
                model->bones[uniqueBoneCount].parent = -1;

                model->boneOffsets[uniqueBoneCount] = AssimpCast<NX_Mat4>(bone->mOffsetMatrix);

                const aiNode* boneNode = findNodeByName(mImporter.rootNode(), bone->mName.data);
                if (boneNode) {
                    aiMatrix4x4 globalTransform = getGlobalNodeTransform(boneNode, mImporter.rootNode());
                    model->boneBindPose[uniqueBoneCount] = AssimpCast<NX_Mat4>(globalTransform);
                }
                else {
                    aiMatrix4x4 bindPoseMatrix = bone->mOffsetMatrix;
                    model->boneBindPose[uniqueBoneCount] = AssimpCast<NX_Mat4>(bindPoseMatrix.Inverse());
                }

                uniqueBoneCount++;
            }
        }
    }

    model->boneCount = uniqueBoneCount;

    /* --- Shrink arrays to actual bone count --- */

    if (uniqueBoneCount < maxPossibleBones) {
        void* boneOffsets = SDL_realloc(model->boneOffsets, uniqueBoneCount * sizeof(NX_Mat4));
        void* boneBindPose = SDL_realloc(model->boneBindPose, uniqueBoneCount * sizeof(NX_Mat4));
        void* bones = SDL_realloc(model->bones, uniqueBoneCount * sizeof(NX_BoneInfo));
        if (boneOffsets) model->boneOffsets = static_cast<NX_Mat4*>(boneOffsets);
        if (boneBindPose) model->boneBindPose = static_cast<NX_Mat4*>(boneBindPose);
        if (bones) model->bones = static_cast<NX_BoneInfo*>(bones);
    }

    /* --- Build bone hierarchy from scene graph --- */

    buildHierarchyRecursive(mImporter.rootNode(), model->bones, model->boneCount, -1);

    return true;
}

/* === Private Implementation === */

inline int BoneImporter::findBoneIndex(const char* name, NX_BoneInfo* bones, int count)
{
    for (int i = 0; i < count; i++) {
        if (SDL_strcmp(name, bones[i].name) == 0) return i;
    }
    return -1;
}

inline aiMatrix4x4 BoneImporter::getGlobalNodeTransform(const aiNode* node, const aiNode* rootNode)
{
    aiMatrix4x4 globalTransform;
    const aiNode* current = node;

    while (current && current != rootNode->mParent) {
        globalTransform = current->mTransformation * globalTransform;
        current = current->mParent;
    }

    return globalTransform;
}

inline const aiNode* BoneImporter::findNodeByName(const aiNode* node, const char* name)
{
    if (SDL_strcmp(node->mName.data, name) == 0) return node;

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        const aiNode* found = findNodeByName(node->mChildren[i], name);
        if (found) return found;
    }

    return nullptr;
}

inline void BoneImporter::buildHierarchyRecursive(const aiNode* node, NX_BoneInfo* bones, int boneCount, int parentIndex)
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

} // namespace import

#endif // NX_IMPORT_BONE_IMPORTER_HPP
