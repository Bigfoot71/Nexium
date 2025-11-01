/* NX_Cubemap.cpp -- Implementation of the API for cubemaps
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Cubemap.hpp"

#include "../Detail/Util/DynamicArray.hpp"
#include "../Detail/GPU/Translation.hpp"
#include "./Core/Helper.hpp"

/* === Public Implementation === */

void NX_Cubemap::generateSkybox(const NX_Skybox& skybox, gpu::Program& programSkyboxGen)
{
    if (!mFramebuffer.isValid()) {
        mFramebuffer = gpu::Framebuffer({&mTexture}, nullptr);
    }

    gpu::Pipeline pipeline;

    pipeline.bindFramebuffer(mFramebuffer);
    pipeline.setViewport(mFramebuffer);

    pipeline.useProgram(programSkyboxGen);

    pipeline.setUniformFloat3(1, NX_Vec3Normalize(-skybox.sunDirection));
    pipeline.setUniformFloat3(2, skybox.skyColorTop);
    pipeline.setUniformFloat3(3, skybox.skyColorHorizon);
    pipeline.setUniformFloat3(4, skybox.sunColor);
    pipeline.setUniformFloat3(5, skybox.groundColor);
    pipeline.setUniformFloat1(6, skybox.sunSize);
    pipeline.setUniformFloat1(7, skybox.haze);
    pipeline.setUniformFloat1(8, skybox.energy);
    pipeline.setUniformInt1(9, mTexture.isHDR());

    for (int i = 0; i < 6; i++) {
        mFramebuffer.setColorAttachmentTarget(0, 0, i);
        pipeline.setUniformMat4(0, render::getCubeView(i) * render::getCubeProj());
        pipeline.draw(GL_TRIANGLES, 36);
    }
}

/* === Private Implementation === */

void NX_Cubemap::loadEquirectangular(const NX_Image& image, gpu::Program& programEquirectangular)
{
    /* --- Determines the internal source and destination formats --- */

    GLenum srcInternalFormat = gpu::getInternalFormat(image.format, false);
    GLenum dstInternalFormat = gpu::getInternalFormat(image.format, true);

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = dstInternalFormat,
            .data = nullptr,
            .width = image.h,
            .height = image.h
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    /* --- Load panorama texture --- */

    gpu::Texture panorama(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_2D,
            .internalFormat = srcInternalFormat,
            .data = image.pixels,
            .width = image.w,
            .height = image.h
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE
        }
    );

    /* --- Convert panorama to cubemap --- */

    gpu::Framebuffer fb({&mTexture});

    gpu::Pipeline pipeline;

    pipeline.bindFramebuffer(fb);
    pipeline.setViewport(fb);

    pipeline.bindTexture(0, panorama);
    pipeline.useProgram(programEquirectangular);

    for (int i = 0; i < 6; i++) {
        fb.setColorAttachmentTarget(0, 0, i);
        pipeline.setUniformInt1(0, i);
        pipeline.draw(GL_TRIANGLES, 3);
    }
}

void NX_Cubemap::loadLineHorizontal(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 6;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = gpu::getInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    /* --- Upload to cubemap --- */

    int bytesPerPixel = NX_GetPixelBytes(image.format);
    const uint8_t* pixels = static_cast<const uint8_t*>(image.pixels);
    util::DynamicArray<uint8_t> faceBuffer(cubeFaceSize * cubeFaceSize * bytesPerPixel);

    gpu::UploadRegion region{
        .x = 0,
        .y = 0,
        .z = 0,
        .width = cubeFaceSize,
        .height = cubeFaceSize,
        .depth = 1,
        .level = 0
    };

    for (int i = 0; i < 6; i++) {
        for (int y = 0; y < cubeFaceSize; y++) {
            const uint8_t* srcRow = pixels + (y * image.w + i * cubeFaceSize) * bytesPerPixel;
            uint8_t* dstRow = faceBuffer.data() + y * cubeFaceSize * bytesPerPixel;
            SDL_memcpy(dstRow, srcRow, cubeFaceSize * bytesPerPixel);
        }
        region.cubeFace = gpu::CubeFace(int(gpu::CubeFace::PositiveX) + i);
        mTexture.upload(faceBuffer.data(), region);
    }
}

void NX_Cubemap::loadLineVertical(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.h / 6;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = gpu::getInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    /* --- Upload to cubemap --- */

    int bytesPerPixel = NX_GetPixelBytes(image.format);
    const uint8_t* pixels = static_cast<const uint8_t*>(image.pixels);

    gpu::UploadRegion region{
        .x = 0,
        .y = 0,
        .z = 0,
        .width = cubeFaceSize,
        .height = cubeFaceSize,
        .depth = 1,
        .level = 0
    };

    for (int i = 0; i < 6; i++) {
        region.cubeFace = gpu::CubeFace(int(gpu::CubeFace::PositiveX) + i);
        mTexture.upload(pixels + (i * cubeFaceSize * image.w * bytesPerPixel), region);
    }
}

void NX_Cubemap::loadCrossThreeByFour(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 3;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = gpu::getInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    /* --- Upload to cubemap --- */

    // Layout 3x4 cross:
    //     [+Y]
    // [-X][+Z][+X]
    //     [-Y]
    //     [-Z]

    struct FacePosition {
        gpu::CubeFace face;
        int x, y;
    };

    const FacePosition facePositions[] = {
        {gpu::CubeFace::PositiveY, 1, 0},  // Top
        {gpu::CubeFace::NegativeX, 0, 1},  // Left
        {gpu::CubeFace::PositiveZ, 1, 1},  // Front
        {gpu::CubeFace::PositiveX, 2, 1},  // Right
        {gpu::CubeFace::NegativeY, 1, 2},  // Bottom
        {gpu::CubeFace::NegativeZ, 1, 3}   // Back
    };

    int bytesPerPixel = NX_GetPixelBytes(image.format);
    const uint8_t* pixels = static_cast<const uint8_t*>(image.pixels);
    util::DynamicArray<uint8_t> faceBuffer(cubeFaceSize * cubeFaceSize * bytesPerPixel);

    for (const auto& pos : facePositions) {
        if (pos.x * cubeFaceSize + cubeFaceSize <= image.w && pos.y * cubeFaceSize + cubeFaceSize <= image.h) {
            for (int y = 0; y < cubeFaceSize; y++) {
                const uint8_t* srcRow = pixels + ((pos.y * cubeFaceSize + y) * image.w + pos.x * cubeFaceSize) * bytesPerPixel;
                uint8_t* dstRow = faceBuffer.data() + y * cubeFaceSize * bytesPerPixel;
                memcpy(dstRow, srcRow, cubeFaceSize * bytesPerPixel);
            }
            mTexture.upload(faceBuffer.data(), gpu::UploadRegion{
                .x = 0, .y = 0, .z = 0,
                .width = cubeFaceSize,
                .height = cubeFaceSize,
                .depth = 1,
                .level = 0,
                .cubeFace = pos.face
            });
        }
    }
}

void NX_Cubemap::loadCrossFourByThree(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 4;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = gpu::getInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE
        }
    );

    /* --- Upload to cubemap --- */

    // Layout 4x3 cross:
    //     [+Y]
    // [-X][+Z][+X][-Z]
    //     [-Y]

    struct FacePosition {
        gpu::CubeFace face;
        int x, y;
    };

    const FacePosition facePositions[] = {
        {gpu::CubeFace::PositiveY, 1, 0},  // Top
        {gpu::CubeFace::NegativeX, 0, 1},  // Left
        {gpu::CubeFace::PositiveZ, 1, 1},  // Front
        {gpu::CubeFace::PositiveX, 2, 1},  // Right
        {gpu::CubeFace::NegativeZ, 3, 1},  // Back
        {gpu::CubeFace::NegativeY, 1, 2}   // Bottom
    };

    int bytesPerPixel = NX_GetPixelBytes(image.format);
    const uint8_t* pixels = static_cast<const uint8_t*>(image.pixels);
    util::DynamicArray<uint8_t> faceBuffer(cubeFaceSize * cubeFaceSize * bytesPerPixel);

    for (const auto& pos : facePositions) {
        if (pos.x * cubeFaceSize + cubeFaceSize <= image.w && pos.y * cubeFaceSize + cubeFaceSize <= image.h) {
            for (int y = 0; y < cubeFaceSize; y++) {
                const uint8_t* srcRow = pixels + ((pos.y * cubeFaceSize + y) * image.w + pos.x * cubeFaceSize) * bytesPerPixel;
                uint8_t* dstRow = faceBuffer.data() + y * cubeFaceSize * bytesPerPixel;
                memcpy(dstRow, srcRow, cubeFaceSize * bytesPerPixel);
            }
            mTexture.upload(faceBuffer.data(), gpu::UploadRegion{
                .x = 0, .y = 0, .z = 0,
                .width = cubeFaceSize,
                .height = cubeFaceSize,
                .depth = 1,
                .level = 0,
                .cubeFace = pos.face
            });
        }
    }
}
