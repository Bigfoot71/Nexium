#ifndef NX_IMPORT_SKELETON_IMPORTER_HPP
#define NX_IMPORT_SKELETON_IMPORTER_HPP

#include <NX/NX_Skeleton.h>
#include <NX/NX_Memory.h>

#include "../INX_GlobalPool.hpp"
#include "./SceneImporter.hpp"
#include "./AssimpHelper.hpp"

#include <SDL3/SDL_assert.h>
#include <assimp/mesh.h>
#include <float.h>

namespace import {

/* === Declaration === */

class SkeletonImporter {
public:
    /** Constructors */
    SkeletonImporter(const SceneImporter& importer);

    /** Loads the bones and stores them in the specified skeleton */
    NX_Skeleton* ProcessSkeleton();

private:
    static int FindBoneIndex(const char* name, NX_BoneInfo* bones, int count);
    static aiMatrix4x4 GetGlobalNodeTransform(const aiNode* node, const aiNode* rootNode);
    static const aiNode* FindNodeByName(const aiNode* node, const char* name);
    static void BuildHierarchyRecursive(const aiNode* node, NX_BoneInfo* bones, int boneCount, int parentIndex);

private:
    const SceneImporter& mImporter;
};

/* === Public Implementation === */

inline SkeletonImporter::SkeletonImporter(const SceneImporter& importer)
    : mImporter(importer)
{
    SDL_assert(importer.isValid());
}

inline NX_Skeleton* SkeletonImporter::ProcessSkeleton()
{
    /* --- Count maximum possible bones across all meshes --- */

    int maxPossibleBones = 0;
    for (uint32_t i = 0; i < mImporter.meshCount(); i++) {
        maxPossibleBones += mImporter.mesh(i)->mNumBones;
    }

    /* --- Early exit if no bones found --- */

    if (maxPossibleBones == 0) {
        return nullptr;
    }

    /* --- Allocate bone arrays --- */

    NX_BoneInfo* bones = NX_Malloc<NX_BoneInfo>(maxPossibleBones);
    NX_Mat4* boneOffsets = NX_Malloc<NX_Mat4>(maxPossibleBones);
    NX_Mat4* boneBindPose = NX_Malloc<NX_Mat4>(maxPossibleBones);

    if (!bones || !boneOffsets || !boneBindPose) {
        NX_LOG(E, "RENDER: Failed to allocate memory for skeleton bones");
        NX_Free(boneBindPose);
        NX_Free(boneOffsets);
        NX_Free(bones);
        return nullptr;
    }

    /* --- Collect unique bones with offset and bind pose matrices --- */

    int uniqueBoneCount = 0;
    for (uint32_t m = 0; m < mImporter.meshCount(); m++) {
        const aiMesh* mesh = mImporter.mesh(m);
        for (uint32_t b = 0; b < mesh->mNumBones; b++) {
            const aiBone* bone = mesh->mBones[b];
            if (FindBoneIndex(bone->mName.data, bones, uniqueBoneCount) == -1)
            {
                SDL_strlcpy(bones[uniqueBoneCount].name, bone->mName.data, sizeof(bones[uniqueBoneCount].name));
                bones[uniqueBoneCount].parent = -1;

                boneOffsets[uniqueBoneCount] = AssimpCast<NX_Mat4>(bone->mOffsetMatrix);

                const aiNode* boneNode = FindNodeByName(mImporter.rootNode(), bone->mName.data);
                if (boneNode) {
                    aiMatrix4x4 globalTransform = GetGlobalNodeTransform(boneNode, mImporter.rootNode());
                    boneBindPose[uniqueBoneCount] = AssimpCast<NX_Mat4>(globalTransform);
                }
                else {
                    aiMatrix4x4 bindPoseMatrix = bone->mOffsetMatrix;
                    boneBindPose[uniqueBoneCount] = AssimpCast<NX_Mat4>(bindPoseMatrix.Inverse());
                }

                uniqueBoneCount++;
            }
        }
    }

    /* --- Shrink arrays to actual bone count --- */

    if (uniqueBoneCount < maxPossibleBones)
    {
        NX_BoneInfo* resizedBones = NX_Realloc<NX_BoneInfo>(bones, uniqueBoneCount);
        if (resizedBones) bones = resizedBones;

        NX_Mat4* resizedBoneOffsets = NX_Realloc<NX_Mat4>(boneOffsets, uniqueBoneCount);
        if (resizedBoneOffsets) boneOffsets = resizedBoneOffsets;

        NX_Mat4* resizedBoneBindPose = NX_Realloc<NX_Mat4>(boneBindPose, uniqueBoneCount);
        if (resizedBoneBindPose) boneBindPose = resizedBoneBindPose;
    }

    /* --- Build bone hierarchy from scene graph --- */

    BuildHierarchyRecursive(mImporter.rootNode(), bones, uniqueBoneCount, -1);

    /* --- Create and return the skeleton --- */

    NX_Skeleton* skeleton = INX_Pool.Create<NX_Skeleton>();

    skeleton->bones = bones;
    skeleton->boneCount = uniqueBoneCount;

    skeleton->boneOffsets = boneOffsets;
    skeleton->boneBindPose = boneBindPose;

    return skeleton;
}

/* === Private Implementation === */

inline int SkeletonImporter::FindBoneIndex(const char* name, NX_BoneInfo* bones, int count)
{
    for (int i = 0; i < count; i++) {
        if (SDL_strcmp(name, bones[i].name) == 0) return i;
    }
    return -1;
}

inline aiMatrix4x4 SkeletonImporter::GetGlobalNodeTransform(const aiNode* node, const aiNode* rootNode)
{
    aiMatrix4x4 globalTransform;
    const aiNode* current = node;

    while (current && current != rootNode->mParent) {
        globalTransform = current->mTransformation * globalTransform;
        current = current->mParent;
    }

    return globalTransform;
}

inline const aiNode* SkeletonImporter::FindNodeByName(const aiNode* node, const char* name)
{
    if (SDL_strcmp(node->mName.data, name) == 0) return node;

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        const aiNode* found = FindNodeByName(node->mChildren[i], name);
        if (found) return found;
    }

    return nullptr;
}

inline void SkeletonImporter::BuildHierarchyRecursive(const aiNode* node, NX_BoneInfo* bones, int boneCount, int parentIndex)
{
    int currentIndex = FindBoneIndex(node->mName.data, bones, boneCount);

    if (currentIndex != -1) {
        bones[currentIndex].parent = parentIndex;
        parentIndex = currentIndex;
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        BuildHierarchyRecursive(node->mChildren[i], bones, boneCount, parentIndex);
    }
}

} // namespace import

#endif // NX_IMPORT_SKELETON_IMPORTER_HPP
