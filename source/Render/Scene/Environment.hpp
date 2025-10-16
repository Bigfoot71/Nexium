#ifndef NX_SCENE_ENVIRONMENT_HPP
#define NX_SCENE_ENVIRONMENT_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Macros.h>

#include "../../Detail/Util/DynamicArray.hpp"
#include "../../Detail/GPU/Buffer.hpp"

namespace scene {

/* === Declaration === */

class Environment {
public:
    /** Constructors */
    Environment();

    /** Update methods */
    void update(const NX_Environment& env, int bloomMipCount);

    /** Textures */
    const NX_Cubemap* skyCubemap() const;
    const NX_ReflectionProbe* skyProbe() const;

    /** CPU data */
    const util::DynamicArray<float>& bloomLevels() const;
    bool hasFlags(NX_EnvironmentFlag flags) const;
    const NX_Color& background() const;
    NX_Tonemap tonemapMode() const;
    NX_Bloom bloomMode() const;
    bool isSsaoEnabled() const;

    /** GPU data */
    const gpu::Buffer& buffer() const;

private:
    NX_Vec4 getBloomPrefilter(float threshold, float softThreshold);

private:
    struct GPUData {
        alignas(16) NX_Vec3 ambientColor;
        alignas(16) NX_Vec4 skyRotation;
        alignas(16) NX_Vec3 fogColor;
        alignas(16) NX_Vec4 bloomPrefilter;
        alignas(4) float skyIntensity;
        alignas(4) float skySpecular;
        alignas(4) float skyDiffuse;
        alignas(4) float fogDensity;
        alignas(4) float fogStart;
        alignas(4) float fogEnd;
        alignas(4) float fogSkyAffect;
        alignas(4) int fogMode;
        alignas(4) float ssaoIntensity;
        alignas(4) float ssaoRadius;
        alignas(4) float ssaoPower;
        alignas(4) float ssaoBias;
        alignas(4) int ssaoEnabled;
        alignas(4) float bloomFilterRadius;
        alignas(4) float bloomStrength;
        alignas(4) int bloomMode;
        alignas(4) float adjustBrightness;
        alignas(4) float adjustContrast;
        alignas(4) float adjustSaturation;
        alignas(4) float tonemapExposure;
        alignas(4) float tonemapWhite;
        alignas(4) int tonemapMode;
    };

private:
    /** Textures */
    NX_Cubemap* mSkyCubemap;
    NX_ReflectionProbe* mSkyProbe;

    /** Scene data */
    NX_EnvironmentFlag mFlags;
    NX_Color mBackground;

    /** Post processing data */
    util::DynamicArray<float> mBloomLevels;
    NX_Tonemap mTonemapMode;
    NX_Bloom mBloomMode;
    bool mSsaoEnabled;

    /** GPU data */
    gpu::Buffer mBuffer;
};

/* === Public Implementation === */

inline Environment::Environment()
    : mBuffer(GL_UNIFORM_BUFFER, sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW)
{ }

inline void Environment::update(const NX_Environment& env, int bloomMipCount)
{
    /* --- Store textures --- */

    mSkyCubemap = env.sky.cubemap;
    mSkyProbe = env.sky.probe;

    /* --- Store CPU data */

    mFlags = env.flags;
    mBackground = env.background;

    // Pre-multiply background with fog
    if (env.fog.mode != NX_FOG_DISABLED) {
        mBackground = NX_ColorLerp(mBackground, env.fog.color, env.fog.skyAffect);
    }

    // Calculation of physical bloom level factors
    if (env.bloom.mode != NX_BLOOM_DISABLED) {
        mBloomLevels.clear();
        if (!mBloomLevels.reserve(bloomMipCount)) {
            NX_INTERNAL_LOG(E, "RENDER: Bloom mip factor buffer reservation failed (requested: %d levels)", bloomMipCount);
        }
        for (uint32_t i = 0; i < bloomMipCount; ++i) {
            float t = float(i) / float(bloomMipCount - 1); // 0 -> 1
            float mapped = t * (int(NX_ARRAY_SIZE(env.bloom.levels)) - 1);
            uint32_t idx0 = uint32_t(mapped);
            uint32_t idx1 = NX_MIN(idx0 + 1, uint32_t(NX_ARRAY_SIZE(env.bloom.levels) - 1));
            float frac = mapped - idx0;
            mBloomLevels.emplace_back(
                env.bloom.levels[idx0] * (1.0f - frac) +
                env.bloom.levels[idx1] * frac
            );
        }
    }

    mTonemapMode = env.tonemap.mode;
    mSsaoEnabled = env.ssao.enabled;
    mBloomMode = env.bloom.mode;

    /* --- Get all GPU data --- */

    GPUData data{};

    data.ambientColor = NX_VEC3(env.ambient.r, env.ambient.g, env.ambient.b);
    data.skyRotation = NX_VEC4(env.sky.rotation.x, env.sky.rotation.y, env.sky.rotation.z, env.sky.rotation.w);
    data.skyIntensity = env.sky.intensity;
    data.skySpecular = env.sky.specular * env.sky.intensity;
    data.skyDiffuse = env.sky.diffuse * env.sky.intensity;

    data.fogDensity = env.fog.density;
    data.fogStart = env.fog.start;
    data.fogEnd = env.fog.end;
    data.fogSkyAffect = (env.fog.mode) ? env.fog.skyAffect : 0.0f;
    data.fogColor = NX_VEC3(env.fog.color.r, env.fog.color.g, env.fog.color.b);
    data.fogMode = env.fog.mode;

    data.ssaoIntensity = env.ssao.intensity;
    data.ssaoRadius = env.ssao.radius;
    data.ssaoPower = env.ssao.power;
    data.ssaoBias = env.ssao.bias;
    data.ssaoEnabled = env.ssao.enabled;

    data.bloomPrefilter = getBloomPrefilter(env.bloom.threshold, env.bloom.softThreshold);
    data.bloomFilterRadius = env.bloom.filterRadius;
    data.bloomStrength = env.bloom.strength;
    data.bloomMode = env.bloom.mode;

    data.adjustBrightness = env.adjustment.brightness;
    data.adjustContrast = env.adjustment.contrast;
    data.adjustSaturation = env.adjustment.saturation;

    data.tonemapExposure = env.tonemap.exposure;
    data.tonemapWhite = env.tonemap.white;
    data.tonemapMode = env.tonemap.mode;

    /* --- Upload GPU data --- */

    mBuffer.upload(&data);
}

inline const NX_Cubemap* Environment::skyCubemap() const
{
    return mSkyCubemap;
}

inline const NX_ReflectionProbe* Environment::skyProbe() const
{
    return mSkyProbe;
}

inline const util::DynamicArray<float>& Environment::bloomLevels() const
{
    return mBloomLevels;
}

inline bool Environment::hasFlags(NX_EnvironmentFlag flags) const
{
    return (mFlags & flags) == flags;
}

inline const NX_Color& Environment::background() const
{
    return mBackground;
}

inline NX_Tonemap Environment::tonemapMode() const
{
    return mTonemapMode;
}

inline NX_Bloom Environment::bloomMode() const
{
    return mBloomMode;
}

inline bool Environment::isSsaoEnabled() const
{
    return mSsaoEnabled;
}

inline const gpu::Buffer& Environment::buffer() const
{
    return mBuffer;
}

/* === Private Implementation === */

inline NX_Vec4 Environment::getBloomPrefilter(float threshold, float softThreshold)
{
    float knee = threshold * softThreshold;

    NX_Vec4 prefilter;
    prefilter.x = threshold;
    prefilter.y = threshold - knee;
    prefilter.z = 2.0f * knee;
    prefilter.w = 0.25f / (knee + 1e-6f);

    return prefilter;
}

} // namespace scene

#endif // NX_SCENE_ENVIRONMENT_HPP
