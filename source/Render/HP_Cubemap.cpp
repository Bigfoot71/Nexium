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

#include "./HP_Cubemap.hpp"
#include "./Core/Helper.hpp"
#include "Hyperion/HP_Image.h"

/* === Private Implementation === */

void HP_Cubemap::loadEquirectangular(const HP_Image& image, gpu::Program& programEquirectangular)
{
    /* --- Determines the internal source and destination formats --- */

    GLenum srcInternalFormat = render::getInternalFormat(image.format);
    GLenum dstInternalFormat = srcInternalFormat;

    // REVIEW: On some emulated GLES 3.2 contexts (e.g. NVIDIA desktop drivers),
    // the extension GL_EXT_color_buffer_float may be reported as supported,
    // but attempting to use 32-bit float color attachments (GL_RGBA32F, etc.)
    // can result in incomplete framebuffers...
    //
    // For maximum compatibility across all GLES 3.2 implementations,
    // we currently force 16-bit float formats (GL_RGBA16F, etc.) for FBO color attachments.
    // This behavior may need to be revisited later.

    if (gCore->glProfile() == SDL_GL_CONTEXT_PROFILE_ES /*&& !GLAD_GL_EXT_color_buffer_float*/) {
        switch (image.format) {
        case HP_PIXEL_FORMAT_R32F: dstInternalFormat = GL_R16F; break;
        case HP_PIXEL_FORMAT_RG32F: dstInternalFormat = GL_RG16F; break;
        case HP_PIXEL_FORMAT_RGB32F: dstInternalFormat = GL_RGB16F; break;
        case HP_PIXEL_FORMAT_RGBA32F: dstInternalFormat = GL_RGBA16F; break;
        default: break;
        }
    }

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

void HP_Cubemap::loadLineHorizontal(const HP_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 6;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = render::getInternalFormat(image.format),
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

    int bytesPerPixel = HP_GetPixelBytes(image.format);
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

void HP_Cubemap::loadLineVertical(const HP_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.h / 6;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = render::getInternalFormat(image.format),
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

    int bytesPerPixel = HP_GetPixelBytes(image.format);
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

void HP_Cubemap::loadCrossThreeByFour(const HP_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 3;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = render::getInternalFormat(image.format),
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

    int bytesPerPixel = HP_GetPixelBytes(image.format);
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

void HP_Cubemap::loadCrossFourByThree(const HP_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 4;

    /* --- Allocate cubemap texture --- */

    mTexture = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = render::getInternalFormat(image.format),
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

    int bytesPerPixel = HP_GetPixelBytes(image.format);
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
