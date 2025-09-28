/* HP_ReflectionProbe.hpp -- Implementation of the API for reflection probes
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_REFLECTION_PROBE_HPP
#define HP_REFLECTION_PROBE_HPP

#include "./HP_Cubemap.hpp"

/* === Declaration === */

class HP_ReflectionProbe {
public:
    HP_ReflectionProbe(const HP_Cubemap& cubemap, gpu::Program& programIrradiance, gpu::Program& programPrefilter);

    bool isValid() const;
    const gpu::Texture& irradiance() const;
    const gpu::Texture& prefilter() const;

    void update(const HP_Cubemap& cubemap, gpu::Program& programIrradiance, gpu::Program& programPrefilter);

private:
    void genIrradiance(const HP_Cubemap& cubemap, gpu::Program& programIrradiance);
    void genPrefilter(const HP_Cubemap& cubemap, gpu::Program& programPrefilter);

private:
    /** Textures */
    gpu::Texture mIrradiance;
    gpu::Texture mPrefilter;

    /** Framebuffers */
    gpu::Framebuffer mFBIrradiance;
    gpu::Framebuffer mFBPrefilter;
};

/* === Public Implementation === */

inline bool HP_ReflectionProbe::isValid() const
{
    return mIrradiance.isValid() && mPrefilter.isValid();
}

inline const gpu::Texture& HP_ReflectionProbe::irradiance() const
{
    return mIrradiance;
}

inline const gpu::Texture& HP_ReflectionProbe::prefilter() const
{
    return mPrefilter;
}

inline void HP_ReflectionProbe::update(const HP_Cubemap& cubemap, gpu::Program& programIrradiance, gpu::Program& programPrefilter)
{
    genIrradiance(cubemap, programIrradiance);
    genPrefilter(cubemap, programPrefilter);
}

#endif // HP_REFLECTION_PROBE_HPP
