#ifndef INX_POOL_ASSETS_HPP
#define INX_POOL_ASSETS_HPP

#include "./Detail/Util/ObjectPool.hpp"
#include "./NX_RenderTexture.hpp"
#include "./NX_AudioStream.hpp"
#include "./NX_AudioClip.hpp"
#include "./NX_Shader3D.hpp"
#include "./NX_Shader2D.hpp"
#include "./NX_Texture.hpp"
#include "./NX_Font.hpp"

// ============================================================================
// ASSETS POOL
// ============================================================================

extern class INX_PoolAssets {
public:
    /** Audio */
    using AudioStreams   = util::ObjectPool<NX_AudioStream, 128>;
    using AudioClips     = util::ObjectPool<NX_AudioClip, 128>;

    /** Render */
    using RenderTextures = util::ObjectPool<NX_RenderTexture, 16>;
    using Textures       = util::ObjectPool<NX_Texture, 1024>;
    using Fonts          = util::ObjectPool<NX_Font, 32>;

    /** Shaders */
    using Shaders3D      = util::ObjectPool<NX_Shader3D, 32>;
    using Shaders2D      = util::ObjectPool<NX_Shader2D, 32>;

public:
    /** Singleton */
    static INX_PoolAssets& Instance();

public:
    template<typename T>
    auto& Get();

    template<typename T, typename... Args>
    T* Create(Args&&... args);

    template<typename T>
    void Destroy(T* object);

    template<typename T, typename F>
    void ForEach(F&& func);

    void Unload();

private:
    /** Audio */
    AudioStreams   mAudioStreams;
    AudioClips     mAudioClips;

    /** Render */
    RenderTextures mRenderTextures;
    Textures       mTextures;
    Fonts          mFonts;

    /** Shaders */
    Shaders3D      mShaders3D;
    Shaders2D      mShaders2D;

} INX_Pool;

template<typename T>
inline auto& INX_PoolAssets::Get()
{
    if constexpr (std::is_same_v<T, NX_AudioStream>)        return mAudioStreams;
    else if constexpr (std::is_same_v<T, NX_AudioClip>)     return mAudioClips;
    else if constexpr (std::is_same_v<T, NX_RenderTexture>) return mRenderTextures;
    else if constexpr (std::is_same_v<T, NX_Texture>)       return mTextures;
    else if constexpr (std::is_same_v<T, NX_Font>)          return mFonts;
    else if constexpr (std::is_same_v<T, NX_Shader3D>)      return mShaders3D;
    else if constexpr (std::is_same_v<T, NX_Shader2D>)      return mShaders2D;
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

inline void INX_PoolAssets::Unload()
{
    mShaders2D.clear();
    mShaders3D.clear();

    mFonts.clear();
    mRenderTextures.clear();
    mTextures.clear();

    mAudioClips.clear();
    mAudioStreams.clear();
}

#endif // INX_POOL_ASSETS_HPP
