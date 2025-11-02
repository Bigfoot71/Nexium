/* INX_PoolAssets.hpp -- Internal implementation details for managing global asset pools
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef INX_POOL_ASSETS_HPP
#define INX_POOL_ASSETS_HPP

#include "./Detail/Util/ObjectPool.hpp"
#include "./NX_ReflectionProbe.hpp"
#include "./NX_InstanceBuffer.hpp"
#include "./NX_RenderTexture.hpp"
#include "./NX_AudioStream.hpp"
#include "./NX_AudioClip.hpp"
#include "./NX_Shader3D.hpp"
#include "./NX_Shader2D.hpp"
#include "./NX_Cubemap.hpp"
#include "./NX_Texture.hpp"
#include "./NX_Font.hpp"

// ============================================================================
// ASSETS POOL
// ============================================================================

extern class INX_PoolAssets {
public:
    /** Audio */
    using AudioStreams      = util::ObjectPool<NX_AudioStream, 128>;
    using AudioClips        = util::ObjectPool<NX_AudioClip, 128>;

    /** Render */
    using ReflectionProbes  = util::ObjectPool<NX_ReflectionProbe, 32>;
    using InstanceBuffers   = util::ObjectPool<NX_InstanceBuffer, 32>;
    using RenderTextures    = util::ObjectPool<NX_RenderTexture, 16>;
    using Textures          = util::ObjectPool<NX_Texture, 1024>;
    using Cubemaps          = util::ObjectPool<NX_Cubemap, 32>;
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
    ReflectionProbes mReflectionProbes;
    InstanceBuffers  mInstanceBuffers;
    RenderTextures   mRenderTextures;
    Textures         mTextures;
    Cubemaps         mCubemaps;
    Fonts            mFonts;

    /** Shaders */
    Shaders3D        mShaders3D;
    Shaders2D        mShaders2D;

} INX_Pool;

template<typename T>
inline auto& INX_PoolAssets::Get()
{
    if constexpr (std::is_same_v<T, NX_AudioStream>)          return mAudioStreams;
    else if constexpr (std::is_same_v<T, NX_AudioClip>)       return mAudioClips;
    else if constexpr (std::is_same_v<T, NX_ReflectionProbe>) return mReflectionProbes;
    else if constexpr (std::is_same_v<T, NX_InstanceBuffer>)  return mInstanceBuffers;
    else if constexpr (std::is_same_v<T, NX_RenderTexture>)   return mRenderTextures;
    else if constexpr (std::is_same_v<T, NX_Texture>)         return mTextures;
    else if constexpr (std::is_same_v<T, NX_Cubemap>)         return mCubemaps;
    else if constexpr (std::is_same_v<T, NX_Font>)            return mFonts;
    else if constexpr (std::is_same_v<T, NX_Shader3D>)        return mShaders3D;
    else if constexpr (std::is_same_v<T, NX_Shader2D>)        return mShaders2D;
    else static_assert(false, "Type not supported by INX_PoolAssets");
}

template<typename T, typename... Args>
inline T* INX_PoolAssets::Create(Args&&... args)
{
    return Get<T>().create(std::forward<Args>(args)...);
}

template<typename T>
inline void INX_PoolAssets::Destroy(T* object)
{
    Get<T>().destroy(object);
}

template<typename T, typename F>
inline void INX_PoolAssets::ForEach(F&& func)
{
    auto& pool = Get<T>();
    for (auto& object : pool) {
        func(object);
    }
}

inline void INX_PoolAssets::UnloadAll()
{
    mShaders2D.clear();
    mShaders3D.clear();

    mFonts.clear();
    mReflectionProbes.clear();
    mInstanceBuffers.clear();
    mRenderTextures.clear();
    mCubemaps.clear();
    mTextures.clear();

    mAudioClips.clear();
    mAudioStreams.clear();
}

#endif // INX_POOL_ASSETS_HPP
