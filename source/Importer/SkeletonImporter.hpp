#ifndef NX_IMPORT_SKELETON_IMPORTER_HPP
#define NX_IMPORT_SKELETON_IMPORTER_HPP

#include <NX/NX_Skeleton.h>
#include <NX/NX_Memory.h>

#include "../INX_GlobalPool.hpp"
#include "./SceneImporter.hpp"
#include "./AssimpHelper.hpp"

#include <SDL3/SDL_assert.h>
#include <assimp/mesh.h>

namespace import {

/* === Declaration === */

class SkeletonImporter {
public:
    SkeletonImporter(const SceneImporter& importer);
    NX_Skeleton* ProcessSkeleton();

private:
    void BuildSkeletonRecursive(
        const aiNode* node, int parentIndex,
        const NX_Mat4& parentTransform);

private:
    const SceneImporter& mImporter;

private:
    /**
     * Owned by the resulting NX_Skeleton, do not free here.
     * Stored as members to share with BuildSkeletonRecursive.
     */
    NX_BoneInfo* mBones{nullptr};
    NX_Mat4* mBoneOffsets{nullptr};
    NX_Mat4* mBindLocal{nullptr};
    NX_Mat4* mBindPose{nullptr};
};

/* === Public Implementation === */

inline SkeletonImporter::SkeletonImporter(const SceneImporter& importer)
    : mImporter(importer)
{
    SDL_assert(importer.IsValid());
}

inline NX_Skeleton* SkeletonImporter::ProcessSkeleton()
{
    int boneCount = mImporter.GetBoneCount();
    if (boneCount == 0) {
        return nullptr;
    }

    /* --- Allocate bone arrays --- */

    mBones = NX_Malloc<NX_BoneInfo>(boneCount);
    mBoneOffsets = NX_Malloc<NX_Mat4>(boneCount);
    mBindLocal = NX_Malloc<NX_Mat4>(boneCount);
    mBindPose = NX_Malloc<NX_Mat4>(boneCount);

    if (!mBones || !mBoneOffsets || !mBindLocal || !mBindPose) {
        NX_LOG(E, "RENDER: Failed to allocate memory for skeleton bones");
        NX_Free(mBoneOffsets);
        NX_Free(mBindLocal);
        NX_Free(mBindPose);
        NX_Free(mBones);
        return nullptr;
    }

    /* --- Initialize parent indices --- */

    for (int i = 0; i < boneCount; i++) {
        mBones[i].parent = -1;
    }

    /* --- Fill bone offsets from meshes --- */

    for (uint32_t m = 0; m < mImporter.GetMeshCount(); m++) {
        const aiMesh* mesh = mImporter.GetMesh(m);
        for (uint32_t b = 0; b < mesh->mNumBones; b++) {
            const aiBone* bone = mesh->mBones[b];
            int boneIdx = mImporter.GetBoneIndex(bone->mName.data);
            if (boneIdx >= 0) {
                mBoneOffsets[boneIdx] = AssimpCast<NX_Mat4>(bone->mOffsetMatrix);
            }
        }
    }

    /* --- Build hierarchy and bind poses in single traversal --- */

    BuildSkeletonRecursive(mImporter.GetRootNode(), -1, NX_MAT4_IDENTITY);

    /* --- Create skeleton --- */

    NX_Skeleton* skeleton = INX_Pool.Create<NX_Skeleton>();

    skeleton->bones = mBones;
    skeleton->boneCount = boneCount;
    skeleton->boneOffsets = mBoneOffsets;
    skeleton->bindLocal = mBindLocal;
    skeleton->bindPose = mBindPose;

    return skeleton;
}

/* === Private Implementation === */

inline void SkeletonImporter::BuildSkeletonRecursive(
    const aiNode* node, int parentIndex,
    const NX_Mat4& parentTransform)
{
    if (!node) return;

    NX_Mat4 globalTransform = AssimpCast<NX_Mat4>(node->mTransformation) * parentTransform;

    int currentIndex = mImporter.GetBoneIndex(node->mName.data);
    if (currentIndex >= 0) {
        mBindPose[currentIndex] = globalTransform;
        mBindLocal[currentIndex] = AssimpCast<NX_Mat4>(node->mTransformation);
        SDL_strlcpy(mBones[currentIndex].name, node->mName.data, sizeof(mBones[currentIndex].name));
        mBones[currentIndex].parent = parentIndex;
        parentIndex = currentIndex;
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        BuildSkeletonRecursive(node->mChildren[i], parentIndex, globalTransform);
    }
}

} // namespace import

#endif // NX_IMPORT_SKELETON_IMPORTER_HPP
