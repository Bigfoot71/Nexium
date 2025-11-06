#ifndef NX_IMPORT_SCENE_IMPORTER_HPP
#define NX_IMPORT_SCENE_IMPORTER_HPP

#include <NX/NX_Log.h>
#include <NX/NX_Math.h>

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <unordered_map>
#include <string>

namespace import {

/* === Declaration === */

class SceneImporter {
public:
    SceneImporter(const void* data, uint32_t size, const char* hint);

    /** Get cache data */
    int GetBoneIndex(const std::string& name) const;

    /** Get assimp data */
    const aiAnimation* GetAnimation(int index) const;
    const aiMaterial* GetMaterial(int index) const;
    const aiTexture* GetTexture(int index) const;
    const aiMesh* GetMesh(int index) const;
    const aiNode* GetRootNode() const;
    const aiScene* GetScene() const;

    /** Get info */
    int GetAnimationCount() const;
    int GetMaterialCount() const;
    int GetTextureCount() const;
    int GetMeshCount() const;
    int GetBoneCount() const;
    bool IsValid() const;

private:
    /** Cache building */
    void BuildBoneMapping();

private:
    /** Assimp data */
    Assimp::Importer mImporter{};
    const aiScene* mScene{};

    /** Cached data */
    std::unordered_map<std::string, int> mBoneIndexMap{};
    int mBoneCount{};
};

/* === Public Implementation === */

inline SceneImporter::SceneImporter(const void* data, uint32_t size, const char* hint)
{
    constexpr uint32_t flags = (
        aiProcess_Triangulate               |
        aiProcess_FlipUVs                   |
        aiProcess_GenNormals                |
        aiProcess_CalcTangentSpace          |
        aiProcess_JoinIdenticalVertices
    );

    mScene = mImporter.ReadFileFromMemory(data, size, flags, hint);
    if (!mScene || !mScene->mRootNode || mScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        NX_LOG(E, "RENDER: Assimp error; %s", mImporter.GetErrorString());
        mImporter.FreeScene();
        mScene = nullptr;
    }

    BuildBoneMapping();
}

inline int SceneImporter::GetBoneIndex(const std::string& name) const
{
    auto it = mBoneIndexMap.find(name);
    if (it != mBoneIndexMap.end()) return it->second;
    return -1;
}

inline const aiAnimation* SceneImporter::GetAnimation(int index) const
{
    return mScene->mAnimations[index];
}

inline const aiMaterial* SceneImporter::GetMaterial(int index) const
{
    return mScene->mMaterials[index];
}

inline const aiTexture* SceneImporter::GetTexture(int index) const
{
    return mScene->mTextures[index];
}

inline const aiMesh* SceneImporter::GetMesh(int index) const
{
    return mScene->mMeshes[index];
}

inline const aiNode* SceneImporter::GetRootNode() const
{
    return mScene->mRootNode;
}

inline const aiScene* SceneImporter::GetScene() const
{
    return mScene;
}

inline int SceneImporter::GetAnimationCount() const
{
    return mScene->mNumAnimations;
}

inline int SceneImporter::GetMaterialCount() const
{
    return mScene->mNumMaterials;
}

inline int SceneImporter::GetTextureCount() const
{
    return mScene->mNumTextures;
}

inline int SceneImporter::GetMeshCount() const
{
    return mScene->mNumMeshes;
}

inline int SceneImporter::GetBoneCount() const
{
    return mBoneCount;
}

inline bool SceneImporter::IsValid() const
{
    return mScene != nullptr;
}

/* === Private Implementation === */

inline void SceneImporter::BuildBoneMapping()
{
    mBoneIndexMap.clear();
    mBoneCount = 0;

    for (uint32_t meshIdx = 0; meshIdx < GetMeshCount(); meshIdx++)
    {
        const aiMesh* mesh = GetMesh(meshIdx);
        if (!mesh || !mesh->HasBones()) continue;

        for (uint32_t boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++)
        {
            const aiBone* bone = mesh->mBones[boneIdx];
            if (!bone) continue;

            const std::string boneName(bone->mName.C_Str());

            if (mBoneIndexMap.find(boneName) == mBoneIndexMap.end()) {
                mBoneIndexMap[boneName] = mBoneCount;
                mBoneCount++;
            }
        }
    }

    if (mBoneCount > 0) {
        NX_LOG(V, "RENDER: Built bone mapping with %d bones", mBoneCount);
    }
}

} // namespace import

#endif // NX_IMPORT_SCENE_IMPORTER_HPP
