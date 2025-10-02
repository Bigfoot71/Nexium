#ifndef HP_SCENE_ENVIRONMENT_HPP
#define HP_SCENE_ENVIRONMENT_HPP

#include <Hyperion/HP_Render.h>
#include "../../Detail/GPU/Buffer.hpp"

namespace scene {

/* === Declaration === */

class Environment {
public:
    Environment();
    void update(const HP_Environment& env);

    /** Textures */
    const HP_Cubemap* skyCubemap() const;
    const HP_ReflectionProbe* skyProbe() const;

    /** CPU data */
    bool hasFlags(HP_EnvironmentFlag flags) const;
    const HP_BoundingBox& bounds() const;
    const HP_Color& background() const;
    HP_Tonemap tonemapMode() const;
    HP_Bloom bloomMode() const;
    bool isSsaoEnabled() const;

    /** GPU data */
    const gpu::Buffer& buffer() const;

private:
    HP_Vec4 getBloomPrefilter(float threshold, float softThreshold);

private:
    struct GPUData {
        alignas(16) HP_Vec3 ambientColor;
        alignas(16) HP_Vec4 skyRotation;
        alignas(16) HP_Vec3 fogColor;
        alignas(16) HP_Vec4 bloomPrefilter;
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
    HP_Cubemap* mSkyCubemap;
    HP_ReflectionProbe* mSkyProbe;

    /** CPU data */
    HP_EnvironmentFlag mFlags;
    HP_BoundingBox mBounds;
    HP_Color mBackground;
    HP_Tonemap mTonemapMode;
    HP_Bloom mBloomMode;
    bool mSsaoEnabled;

    /** GPU data */
    gpu::Buffer mBuffer;
};

/* === Public Implementation === */

inline Environment::Environment()
    : mBuffer(GL_UNIFORM_BUFFER, sizeof(GPUData), nullptr, GL_DYNAMIC_DRAW)
{ }

inline void Environment::update(const HP_Environment& env)
{
    /* --- Store textures --- */

    mSkyCubemap = env.sky.cubemap;
    mSkyProbe = env.sky.probe;

    /* --- Store CPU data */

    mFlags = env.flags;
    mBounds = env.bounds;
    mBackground = env.background;

    if (env.fog.mode != HP_FOG_DISABLED) {
        mBackground = HP_ColorLerp(mBackground, env.fog.color, env.fog.skyAffect);
    }

    mTonemapMode = env.tonemap.mode;
    mBloomMode = env.bloom.mode;
    mSsaoEnabled = env.ssao.enabled;

    /* --- Get all GPU data --- */

    GPUData data{};

    data.ambientColor = HP_VEC3(env.ambient.r, env.ambient.g, env.ambient.b);
    data.skyRotation = HP_VEC4(env.sky.rotation.x, env.sky.rotation.y, env.sky.rotation.z, env.sky.rotation.w);
    data.skyIntensity = env.sky.intensity;
    data.skySpecular = env.sky.specular * env.sky.intensity;
    data.skyDiffuse = env.sky.diffuse * env.sky.intensity;

    data.fogDensity = env.fog.density;
    data.fogStart = env.fog.start;
    data.fogEnd = env.fog.end;
    data.fogSkyAffect = (env.fog.mode) ? env.fog.skyAffect : 0.0f;
    data.fogColor = HP_VEC3(env.fog.color.r, env.fog.color.g, env.fog.color.b);
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

inline const HP_Cubemap* Environment::skyCubemap() const
{
    return mSkyCubemap;
}

inline const HP_ReflectionProbe* Environment::skyProbe() const
{
    return mSkyProbe;
}

inline bool Environment::hasFlags(HP_EnvironmentFlag flags) const
{
    return (mFlags & flags) == flags;
}

inline const HP_BoundingBox& Environment::bounds() const
{
    return mBounds;
}

inline const HP_Color& Environment::background() const
{
    return mBackground;
}

inline HP_Tonemap Environment::tonemapMode() const
{
    return mTonemapMode;
}

inline HP_Bloom Environment::bloomMode() const
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

inline HP_Vec4 Environment::getBloomPrefilter(float threshold, float softThreshold)
{
    float knee = threshold * softThreshold;

    HP_Vec4 prefilter;
    prefilter.x = threshold;
    prefilter.y = threshold - knee;
    prefilter.z = 2.0f * knee;
    prefilter.w = 0.25f / (knee + 1e-6f);

    return prefilter;
}

} // namespace scene

#endif // HP_SCENE_ENVIRONMENT_HPP
