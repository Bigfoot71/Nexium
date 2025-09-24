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

#ifndef HP_RENDER_POOL_CUBEMAP_HPP
#define HP_RENDER_POOL_CUBEMAP_HPP

#include "../../Detail/Util/ObjectPool.hpp"
#include "../../Detail/GPU/Program.hpp"
#include "../HP_ReflectionProbe.hpp"
#include "../HP_Cubemap.hpp"

namespace render {

/* === Declaration === */

class PoolCubemap {
public:
    PoolCubemap(const gpu::Shader& vertScreen, const gpu::Shader& cubeScreen);

public:
    HP_Cubemap* createCubemap(const HP_Image& image);
    void destroyCubemap(HP_Cubemap* cubemap);

    HP_ReflectionProbe* createReflectionProbe(const HP_Cubemap& cubemap);
    void destroyReflectionProbe(HP_ReflectionProbe* probe);

    void updateReflectionProbe(HP_ReflectionProbe* probe, const HP_Cubemap& cubemap);

private:
    util::ObjectPool<HP_ReflectionProbe, 64> mPoolProbes{};
    util::ObjectPool<HP_Cubemap, 64> mPoolCubemaps{};
    gpu::Program mProgramEquirectangular;
    gpu::Program mProgramIrradiance;
    gpu::Program mProgramPrefilter;
};

/* === Public Implementation === */

inline HP_Cubemap* PoolCubemap::createCubemap(const HP_Image& image)
{
    HP_Cubemap* cubemap = mPoolCubemaps.create(image, mProgramEquirectangular);

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

inline HP_ReflectionProbe* PoolCubemap::createReflectionProbe(const HP_Cubemap& cubemap)
{
    HP_ReflectionProbe* probe = mPoolProbes.create(cubemap, mProgramIrradiance, mProgramPrefilter);

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
    probe->update(cubemap, mProgramIrradiance, mProgramPrefilter);
}

} // namespace render

#endif // HP_RENDER_POOL_CUBEMAP_HPP
