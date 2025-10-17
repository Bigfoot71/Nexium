#ifndef NX_RENDER_SCENE_IMPORTER_HPP
#define NX_RENDER_SCENE_IMPORTER_HPP

#include "../../../Core/NX_InternalLog.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace render {

/* === Declaration === */

class SceneImporter {
public:
    SceneImporter(const void* data, uint32_t size, const char* hint);

    /** Get data */
    const aiAnimation* animation(int index) const;
    const aiMaterial* material(int index) const;
    const aiTexture* texture(int index) const;
    const aiMesh* mesh(int index) const;
    const aiNode* rootNode() const;
    const aiScene* scene() const;

    /** Get info */
    int animationCount() const;
    int materialCount() const;
    int textureCount() const;
    int meshCount() const;
    bool isValid() const;

private:
    Assimp::Importer mImporter{};
    const aiScene* mScene{};
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
        NX_INTERNAL_LOG(E, "RENDER: Assimp error; %s", mImporter.GetErrorString());
        mImporter.FreeScene();
        mScene = nullptr;
    }
}

inline const aiAnimation* SceneImporter::animation(int index) const
{
    return mScene->mAnimations[index];
}

inline const aiMaterial* SceneImporter::material(int index) const
{
    return mScene->mMaterials[index];
}

inline const aiTexture* SceneImporter::texture(int index) const
{
    return mScene->mTextures[index];
}

inline const aiMesh* SceneImporter::mesh(int index) const
{
    return mScene->mMeshes[index];
}

inline const aiNode* SceneImporter::rootNode() const
{
    return mScene->mRootNode;
}

inline const aiScene* SceneImporter::scene() const
{
    return mScene;
}

inline int SceneImporter::animationCount() const
{
    return mScene->mNumAnimations;
}

inline int SceneImporter::materialCount() const
{
    return mScene->mNumMaterials;
}

inline int SceneImporter::textureCount() const
{
    return mScene->mNumTextures;
}

inline int SceneImporter::meshCount() const
{
    return mScene->mNumMeshes;
}

inline bool SceneImporter::isValid() const
{
    return mScene != nullptr;
}

} // namespace render

#endif // NX_RENDER_SCENE_IMPORTER_HPP
