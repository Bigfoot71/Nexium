/* PoolCubemap.hpp -- Storage pool for cubemaps and other conceptually related assets
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_POOL_CUBEMAP_HPP
#define NX_RENDER_POOL_CUBEMAP_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Detail/GPU/Program.hpp"
#include "./ProgramCache.hpp"

#include "../NX_ReflectionProbe.hpp"
#include "../NX_Cubemap.hpp"

namespace render {

/* === Declaration === */

class PoolCubemap {
public:
    PoolCubemap(render::ProgramCache& programs);

public:
    NX_Cubemap* createCubemap(int size, NX_PixelFormat format);
    NX_Cubemap* createCubemap(const NX_Image& image);
    void destroyCubemap(NX_Cubemap* cubemap);
    void generateSkybox(NX_Cubemap* cubemap, const NX_Skybox& skybox);

    NX_ReflectionProbe* createReflectionProbe(const NX_Cubemap& cubemap);
    void destroyReflectionProbe(NX_ReflectionProbe* probe);
    void updateReflectionProbe(NX_ReflectionProbe* probe, const NX_Cubemap& cubemap);

private:
    util::ObjectPool<NX_ReflectionProbe, 64> mPoolProbes{};
    util::ObjectPool<NX_Cubemap, 64> mPoolCubemaps{};

private:
    render::ProgramCache& mPrograms;
};

/* === Public Implementation === */

inline PoolCubemap::PoolCubemap(render::ProgramCache& programs)
    : mPrograms(programs)
{ }

inline NX_Cubemap* PoolCubemap::createCubemap(int size, NX_PixelFormat format)
{
    return mPoolCubemaps.create(size, format);
}

inline NX_Cubemap* PoolCubemap::createCubemap(const NX_Image& image)
{
    NX_Cubemap* cubemap = mPoolCubemaps.create(image, mPrograms.cubemapFromEquirectangular());

    if (cubemap == nullptr) {
        NX_LOG(E, "RENDER: Failed to load cubemap; Object pool issue");
        return nullptr;
    }

    if (!cubemap->isValid()) {
        mPoolCubemaps.destroy(cubemap);
        return nullptr;
    }

    return cubemap;
}

inline void PoolCubemap::destroyCubemap(NX_Cubemap* cubemap)
{
    if (cubemap != nullptr) {
        mPoolCubemaps.destroy(cubemap);
    }
}

inline void PoolCubemap::generateSkybox(NX_Cubemap* cubemap, const NX_Skybox& skybox)
{
    cubemap->generateSkybox(skybox, mPrograms.cubemapSkybox());
}

inline NX_ReflectionProbe* PoolCubemap::createReflectionProbe(const NX_Cubemap& cubemap)
{
    NX_ReflectionProbe* probe = mPoolProbes.create(cubemap, mPrograms.cubemapIrradiance(), mPrograms.cubemapPrefilter());

    if (probe == nullptr) {
        NX_LOG(E, "RENDER: Failed to load reflection probe; Object pool issue");
        return nullptr;
    }

    if (!probe->isValid()) {
        mPoolProbes.destroy(probe);
        return nullptr;
    }

    return probe;
}

inline void PoolCubemap::destroyReflectionProbe(NX_ReflectionProbe* probe)
{
    mPoolProbes.destroy(probe);
}

inline void PoolCubemap::updateReflectionProbe(NX_ReflectionProbe* probe, const NX_Cubemap& cubemap)
{
    probe->update(cubemap, mPrograms.cubemapIrradiance(), mPrograms.cubemapPrefilter());
}

} // namespace render

#endif // NX_RENDER_POOL_CUBEMAP_HPP
