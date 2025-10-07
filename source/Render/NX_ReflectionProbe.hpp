/* NX_ReflectionProbe.hpp -- Implementation of the API for reflection probes
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_REFLECTION_PROBE_HPP
#define NX_REFLECTION_PROBE_HPP

#include "./NX_Cubemap.hpp"

/* === Declaration === */

class NX_ReflectionProbe {
public:
    NX_ReflectionProbe(const NX_Cubemap& cubemap, gpu::Program& programIrradiance, gpu::Program& programPrefilter);

    bool isValid() const;
    const gpu::Texture& irradiance() const;
    const gpu::Texture& prefilter() const;

    void update(const NX_Cubemap& cubemap, gpu::Program& programIrradiance, gpu::Program& programPrefilter);

private:
    void genIrradiance(const NX_Cubemap& cubemap, gpu::Program& programIrradiance);
    void genPrefilter(const NX_Cubemap& cubemap, gpu::Program& programPrefilter);

private:
    /** Textures */
    gpu::Texture mIrradiance;
    gpu::Texture mPrefilter;

    /** Framebuffers */
    gpu::Framebuffer mFBIrradiance;
    gpu::Framebuffer mFBPrefilter;
};

/* === Public Implementation === */

inline bool NX_ReflectionProbe::isValid() const
{
    return mIrradiance.isValid() && mPrefilter.isValid();
}

inline const gpu::Texture& NX_ReflectionProbe::irradiance() const
{
    return mIrradiance;
}

inline const gpu::Texture& NX_ReflectionProbe::prefilter() const
{
    return mPrefilter;
}

inline void NX_ReflectionProbe::update(const NX_Cubemap& cubemap, gpu::Program& programIrradiance, gpu::Program& programPrefilter)
{
    genIrradiance(cubemap, programIrradiance);
    genPrefilter(cubemap, programPrefilter);
}

#endif // NX_REFLECTION_PROBE_HPP
