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

#ifndef HP_CUBEMAP_HPP
#define HP_CUBEMAP_HPP

#include <Hyperion/HP_Image.h>

#include "../Detail/GPU/Framebuffer.hpp"
#include "../Detail/GPU/Pipeline.hpp"
#include "../Detail/GPU/Texture.hpp"
#include "./Core/Helper.hpp"

/* === Declaration === */

class HP_Cubemap {
public:
    HP_Cubemap(const HP_Image& image, gpu::Program& programEquirectangular);

    bool isValid() const;
    int mipLevels() const;
    HP_IVec2 dimensions() const;
    const gpu::Texture& texture() const;

private:
    void loadEquirectangular(const HP_Image& image, gpu::Program& programEquirectangular);
    void loadLineHorizontal(const HP_Image& image);
    void loadLineVertical(const HP_Image& image);
    void loadCrossThreeByFour(const HP_Image& image);
    void loadCrossFourByThree(const HP_Image& image);

private:
    gpu::Texture mTexture;
};

/* === Public Implementation === */

inline HP_Cubemap::HP_Cubemap(const HP_Image& image, gpu::Program& programEquirectangular)
{
    /* --- Layout detection and cubemap loading --- */

    if (image.w > image.h) {
        if (image.w == 2 * image.h) {
            loadEquirectangular(image, programEquirectangular);
        }
        else if (image.w / 6 == image.h) {
            loadLineHorizontal(image);
        }
        else if (image.w / 4 == image.h / 3) {
            loadCrossFourByThree(image);
        }
    }
    else if (image.h > image.w) {
        if (image.h / 6 == image.w) {
            loadLineVertical(image);
        }
        else if (image.w / 3 == image.h / 4) {
            loadCrossThreeByFour(image);
        }
    }

    if (!mTexture.isValid()) {
        HP_INTERNAL_LOG(E, "RENDER: Unable to determine skybox cubemap layout");
        return;
    }

    /* --- Generate mipmaps and setup parameters --- */

    mTexture.generateMipmap(); //< Needed for prefilter
    mTexture.setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

inline bool HP_Cubemap::isValid() const
{
    return mTexture.isValid();
}

inline int HP_Cubemap::mipLevels() const
{
    return mTexture.mipLevels();
}

inline HP_IVec2 HP_Cubemap::dimensions() const
{
    return mTexture.dimensions();
}

inline const gpu::Texture& HP_Cubemap::texture() const
{
    return mTexture;
}

#endif // HP_CUBEMAP_HPP
