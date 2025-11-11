/* INX_GlobalPool.hpp -- Internal implementation details for managing global asset pools
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef INX_GLOBAL_POOL_HPP
#define INX_GLOBAL_POOL_HPP

#include <NX/NX_AnimationPlayer.h>
#include <NX/NX_Animation.h>
#include <NX/NX_Skeleton.h>
#include <NX/NX_Model.h>

#include "./Detail/Util/ObjectPool.hpp"
#include "./NX_ReflectionProbe.hpp"
#include "./NX_InstanceBuffer.hpp"
#include "./NX_RenderTexture.hpp"
#include "./NX_DynamicMesh.hpp"
#include "./NX_AudioStream.hpp"
#include "./NX_AudioClip.hpp"
#include "./NX_Shader3D.hpp"
#include "./NX_Shader2D.hpp"
#include "./NX_Cubemap.hpp"
#include "./NX_Texture.hpp"
#include "./NX_Light.hpp"
#include "./NX_Font.hpp"

// ============================================================================
// ASSETS POOL
// ============================================================================

class INX_GlobalPool {
public:
    /** Audio */
    using AudioStreams      = util::ObjectPool<NX_AudioStream, 128>;
    using AudioClips        = util::ObjectPool<NX_AudioClip, 128>;

    /** Render */
    using AnimationPlayers  = util::ObjectPool<NX_AnimationPlayer, 128>;
    using ReflectionProbes  = util::ObjectPool<NX_ReflectionProbe, 128>;
    using VertexBuffers3D   = util::ObjectPool<NX_VertexBuffer3D, 512>;
    using InstanceBuffers   = util::ObjectPool<NX_InstanceBuffer, 32>;
    using RenderTextures    = util::ObjectPool<NX_RenderTexture, 16>;
    using AnimationLibs     = util::ObjectPool<NX_AnimationLib, 256>;
    using DynamicMeshes     = util::ObjectPool<NX_DynamicMesh, 32>;
    using Skeletons         = util::ObjectPool<NX_Skeleton, 128>;
    using Textures          = util::ObjectPool<NX_Texture, 1024>;
    using Cubemaps          = util::ObjectPool<NX_Cubemap, 32>;
    using Lights            = util::ObjectPool<NX_Light, 128>;
    using Models            = util::ObjectPool<NX_Model, 128>;
    using Meshes            = util::ObjectPool<NX_Mesh, 512>;
    using Fonts             = util::ObjectPool<NX_Font, 32>;

    /** Shaders */
    using Shaders3D         = util::ObjectPool<NX_Shader3D, 32>;
    using Shaders2D         = util::ObjectPool<NX_Shader2D, 32>;

public:
    template<typename T>
    auto& Get();

    template<typename T, typename... Args>
    T* Create(Args&&... args);

    template<typename T>
    void Destroy(T* object);

    template<typename T, typename F>
    void ForEach(F&& func);

    void UnloadAll();

private:
    /** Audio */
    AudioStreams     mAudioStreams;
    AudioClips       mAudioClips;

    /** Render */
    AnimationPlayers mAnimationPlayers;
    ReflectionProbes mReflectionProbes;
    VertexBuffers3D  mVertexBuffers3D;
    InstanceBuffers  mInstanceBuffers;
    RenderTextures   mRenderTextures;
    AnimationLibs    mAnimationLibs;
    DynamicMeshes    mDynamicMeshes;
    Skeletons        mSkeletons;
    Textures         mTextures;
    Cubemaps         mCubemaps;
    Models           mModels;
    Meshes           mMeshes;
    Lights           mLights;
    Fonts            mFonts;

    /** Shaders */
    Shaders3D        mShaders3D;
    Shaders2D        mShaders2D;

};

extern INX_GlobalPool INX_Pool;

template<typename T>
inline auto& INX_GlobalPool::Get()
{
    if constexpr (std::is_same_v<T, NX_AudioStream>)          return mAudioStreams;
    else if constexpr (std::is_same_v<T, NX_AudioClip>)       return mAudioClips;
    else if constexpr (std::is_same_v<T, NX_AnimationPlayer>) return mAnimationPlayers;
    else if constexpr (std::is_same_v<T, NX_ReflectionProbe>) return mReflectionProbes;
    else if constexpr (std::is_same_v<T, NX_VertexBuffer3D>)  return mVertexBuffers3D;
    else if constexpr (std::is_same_v<T, NX_InstanceBuffer>)  return mInstanceBuffers;
    else if constexpr (std::is_same_v<T, NX_RenderTexture>)   return mRenderTextures;
    else if constexpr (std::is_same_v<T, NX_AnimationLib>)    return mAnimationLibs;
    else if constexpr (std::is_same_v<T, NX_DynamicMesh>)     return mDynamicMeshes;
    else if constexpr (std::is_same_v<T, NX_Skeleton>)        return mSkeletons;
    else if constexpr (std::is_same_v<T, NX_Texture>)         return mTextures;
    else if constexpr (std::is_same_v<T, NX_Cubemap>)         return mCubemaps;
    else if constexpr (std::is_same_v<T, NX_Model>)           return mModels;
    else if constexpr (std::is_same_v<T, NX_Mesh>)            return mMeshes;
    else if constexpr (std::is_same_v<T, NX_Light>)           return mLights;
    else if constexpr (std::is_same_v<T, NX_Font>)            return mFonts;
    else if constexpr (std::is_same_v<T, NX_Shader3D>)        return mShaders3D;
    else if constexpr (std::is_same_v<T, NX_Shader2D>)        return mShaders2D;
    else static_assert(false, "Type not supported by INX_GlobalPool");
}

template<typename T, typename... Args>
inline T* INX_GlobalPool::Create(Args&&... args)
{
    return Get<T>().Create(std::forward<Args>(args)...);
}

template<typename T>
inline void INX_GlobalPool::Destroy(T* object)
{
    Get<T>().Destroy(object);
}

template<typename T, typename F>
inline void INX_GlobalPool::ForEach(F&& func)
{
    auto& pool = Get<T>();
    for (auto& object : pool) {
        func(object);
    }
}

inline void INX_GlobalPool::UnloadAll()
{
    if (!mShaders2D.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Shader2D objects were not freed! Possible memory leak", mShaders2D.GetSize());
        mShaders2D.Clear();
    }

    if (!mShaders3D.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Shader3D objects were not freed! Possible memory leak", mShaders3D.GetSize());
        mShaders3D.Clear();
    }

    if (!mLights.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Light objects were not freed! Possible memory leak", mLights.GetSize());
        mLights.Clear();
    }

    if (!mModels.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Model objects were not freed! Possible memory leak", mModels.GetSize());
        mModels.Clear();
    }

    if (!mMeshes.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Mesh objects were not freed! Possible memory leak", mMeshes.GetSize());
        mMeshes.Clear();
    }

    if (!mSkeletons.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Skeleton objects were not freed! Possible memory leak", mSkeletons.GetSize());
        mSkeletons.Clear();
    }

    if (!mAnimationPlayers.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_AnimationPlayer objects were not freed! Possible memory leak", mAnimationPlayers.GetSize());
        mAnimationPlayers.Clear();
    }

    if (!mAnimationLibs.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_AnimationLib objects were not freed! Possible memory leak", mAnimationLibs.GetSize());
        mAnimationLibs.Clear();
    }

    if (!mDynamicMeshes.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_DynamicMesh objects were not freed! Possible memory leak", mDynamicMeshes.GetSize());
        mDynamicMeshes.Clear();
    }

    if (!mInstanceBuffers.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_InstanceBuffer objects were not freed! Possible memory leak", mInstanceBuffers.GetSize());
        mInstanceBuffers.Clear();
    }

    if (!mVertexBuffers3D.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_VertexBuffer3D objects were not freed! Possible memory leak", mVertexBuffers3D.GetSize());
        mVertexBuffers3D.Clear();
    }

    if (!mReflectionProbes.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_ReflectionProbe objects were not freed! Possible memory leak", mReflectionProbes.GetSize());
        mReflectionProbes.Clear();
    }

    if (!mRenderTextures.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_RenderTexture objects were not freed! Possible memory leak", mRenderTextures.GetSize());
        mRenderTextures.Clear();
    }

    if (!mCubemaps.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Cubemap objects were not freed! Possible memory leak", mCubemaps.GetSize());
        mCubemaps.Clear();
    }

    if (!mFonts.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Font objects were not freed! Possible memory leak", mFonts.GetSize());
        mFonts.Clear();
    }

    if (!mTextures.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_Texture objects were not freed! Possible memory leak", mTextures.GetSize());
        mTextures.Clear();
    }

    if (!mAudioClips.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_AudioClip objects were not freed! Possible memory leak", mAudioClips.GetSize());
        mAudioClips.Clear();
    }

    if (!mAudioStreams.IsEmpty()) {
        NX_LOG(W, "POOL: %i NX_AudioStream objects were not freed! Possible memory leak", mAudioStreams.GetSize());
        mAudioStreams.Clear();
    }
}

#endif // INX_GLOBAL_POOL_HPP
