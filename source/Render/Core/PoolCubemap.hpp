/* PoolCubemap.hpp -- Storage pool for cubemaps and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_RENDER_POOL_CUBEMAP_HPP
#define HP_RENDER_POOL_CUBEMAP_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Detail/GPU/Program.hpp"
#include "./ProgramCache.hpp"

#include "../HP_ReflectionProbe.hpp"
#include "../HP_Cubemap.hpp"

namespace render {

/* === Declaration === */

class PoolCubemap {
public:
    PoolCubemap(render::ProgramCache& programs);

public:
    HP_Cubemap* createCubemap(int size, HP_PixelFormat format);
    HP_Cubemap* createCubemap(const HP_Image& image);
    void destroyCubemap(HP_Cubemap* cubemap);
    void generateSkybox(HP_Cubemap* cubemap, const HP_Skybox& skybox);

    HP_ReflectionProbe* createReflectionProbe(const HP_Cubemap& cubemap);
    void destroyReflectionProbe(HP_ReflectionProbe* probe);
    void updateReflectionProbe(HP_ReflectionProbe* probe, const HP_Cubemap& cubemap);

private:
    util::ObjectPool<HP_ReflectionProbe, 64> mPoolProbes{};
    util::ObjectPool<HP_Cubemap, 64> mPoolCubemaps{};

private:
    render::ProgramCache& mPrograms;
};

/* === Public Implementation === */

inline PoolCubemap::PoolCubemap(render::ProgramCache& programs)
    : mPrograms(programs)
{ }

inline HP_Cubemap* PoolCubemap::createCubemap(int size, HP_PixelFormat format)
{
    return mPoolCubemaps.create(size, format);
}

inline HP_Cubemap* PoolCubemap::createCubemap(const HP_Image& image)
{
    HP_Cubemap* cubemap = mPoolCubemaps.create(image, mPrograms.cubemapFromEquirectangular());

    if (cubemap == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load cubemap; Object pool issue");
        return nullptr;
    }

    if (!cubemap->isValid()) {
        mPoolCubemaps.destroy(cubemap);
        return nullptr;
    }

    return cubemap;
}

inline void PoolCubemap::destroyCubemap(HP_Cubemap* cubemap)
{
    if (cubemap != nullptr) {
        mPoolCubemaps.destroy(cubemap);
    }
}

inline void PoolCubemap::generateSkybox(HP_Cubemap* cubemap, const HP_Skybox& skybox)
{
    cubemap->generateSkybox(skybox, mPrograms.cubemapSkybox());
}

inline HP_ReflectionProbe* PoolCubemap::createReflectionProbe(const HP_Cubemap& cubemap)
{
    HP_ReflectionProbe* probe = mPoolProbes.create(cubemap, mPrograms.cubemapIrradiance(), mPrograms.cubemapPrefilter());

    if (probe == nullptr) {
        HP_INTERNAL_LOG(E, "RENDER: Failed to load reflection probe; Object pool issue");
        return nullptr;
    }

    if (!probe->isValid()) {
        mPoolProbes.destroy(probe);
        return nullptr;
    }

    return probe;
}

inline void PoolCubemap::destroyReflectionProbe(HP_ReflectionProbe* probe)
{
    mPoolProbes.destroy(probe);
}

inline void PoolCubemap::updateReflectionProbe(HP_ReflectionProbe* probe, const HP_Cubemap& cubemap)
{
    probe->update(cubemap, mPrograms.cubemapIrradiance(), mPrograms.cubemapPrefilter());
}

} // namespace render

#endif // HP_RENDER_POOL_CUBEMAP_HPP
