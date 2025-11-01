#ifndef INX_POOL_ASSETS_HPP
#define INX_POOL_ASSETS_HPP

#include "./Detail/Util/ObjectPool.hpp"
#include "./NX_RenderTexture.hpp"
#include "./NX_AudioStream.hpp"
#include "./NX_AudioClip.hpp"
#include "./NX_Texture.hpp"
#include "./NX_Font.hpp"

// ============================================================================
// ASSETS POOL
// ============================================================================

class INX_PoolAssets {
public:
    /** Audio */
    using AudioStreams   = util::ObjectPool<NX_AudioStream, 128>;
    using AudioClips     = util::ObjectPool<NX_AudioClip, 128>;

    /** Render */
    using RenderTextures = util::ObjectPool<NX_RenderTexture, 16>;
    using Textures       = util::ObjectPool<NX_Texture, 1024>;
    using Fonts          = util::ObjectPool<NX_Font, 32>;

public:
    /** Singleton */
    static INX_PoolAssets& Instance();

public:
    /** Getters */
    template<typename T>
    auto& Get();

    /** Factory-style accessors */
    template<typename T, typename... Args>
    T* Create(Args&&... args);

    template<typename T>
    void Destroy(T* object);

    /** Cleanup */
    void Unload();

private:
    /** Audio */
    AudioStreams   mAudioStreams;
    AudioClips     mAudioClips;

    /** Render */
    RenderTextures mRenderTextures;
    Textures       mTextures;
    Fonts          mFonts;

};

extern INX_PoolAssets INX_Pool;

template<typename T>
inline auto& INX_PoolAssets::Get()
{
    if constexpr (std::is_same_v<T, NX_AudioStream>)        return mAudioStreams;
    else if constexpr (std::is_same_v<T, NX_AudioClip>)     return mAudioClips;
    else if constexpr (std::is_same_v<T, NX_RenderTexture>) return mRenderTextures;
    else if constexpr (std::is_same_v<T, NX_Texture>)       return mTextures;
    else if constexpr (std::is_same_v<T, NX_Font>)          return mFonts;
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

inline void INX_PoolAssets::Unload()
{
    mAudioStreams.clear();
    mAudioClips.clear();
    mRenderTextures.clear();
    mTextures.clear();
    mFonts.clear();
}

#endif // INX_POOL_ASSETS_HPP
