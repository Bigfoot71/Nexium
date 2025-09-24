/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
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
