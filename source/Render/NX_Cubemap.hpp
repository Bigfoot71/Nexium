/* NX_Cubemap.hpp -- Implementation of the API for cubemaps
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_CUBEMAP_HPP
#define NX_CUBEMAP_HPP

#include <NX/NX_Render.h>
#include <NX/NX_Image.h>

#include "../Detail/GPU/Translation.hpp"
#include "../Detail/GPU/Framebuffer.hpp"
#include "../Detail/GPU/Pipeline.hpp"
#include "../Detail/GPU/Texture.hpp"
#include "./Core/Helper.hpp"

/* === Declaration === */

class NX_Cubemap {
public:
    NX_Cubemap(int size, NX_PixelFormat format);
    NX_Cubemap(const NX_Image& image, gpu::Program& programEquirectangular);

    /** Getters */
    bool isValid() const;
    int numLevels() const;
    NX_IVec2 dimensions() const;
    const gpu::Texture& texture() const;

    /** Update methods */
    void generateSkybox(const NX_Skybox& skybox, gpu::Program& programSkyboxGen);

private:
    void loadEquirectangular(const NX_Image& image, gpu::Program& programEquirectangular);
    void loadLineHorizontal(const NX_Image& image);
    void loadLineVertical(const NX_Image& image);
    void loadCrossThreeByFour(const NX_Image& image);
    void loadCrossFourByThree(const NX_Image& image);

private:
    gpu::Texture mTexture;
    gpu::Framebuffer mFramebuffer;  //< Invalid by default, created only if needed
};

/* === Public Implementation === */

inline NX_Cubemap::NX_Cubemap(int size, NX_PixelFormat format)
    : mTexture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = gpu::getInternalFormat(format, true),
            .data = nullptr,
            .width = size,
            .height = size
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    )
    , mFramebuffer(
        {&mTexture},
        nullptr
    )
{ }

inline NX_Cubemap::NX_Cubemap(const NX_Image& image, gpu::Program& programEquirectangular)
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
        NX_LOG(E, "RENDER: Unable to determine skybox cubemap layout");
        return;
    }

    /* --- Generate mipmaps and setup parameters --- */

    mTexture.generateMipmap(); //< Needed for prefilter
    mTexture.setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

inline bool NX_Cubemap::isValid() const
{
    return mTexture.isValid();
}

inline int NX_Cubemap::numLevels() const
{
    return mTexture.numLevels();
}

inline NX_IVec2 NX_Cubemap::dimensions() const
{
    return mTexture.dimensions();
}

inline const gpu::Texture& NX_Cubemap::texture() const
{
    return mTexture;
}

#endif // NX_CUBEMAP_HPP
