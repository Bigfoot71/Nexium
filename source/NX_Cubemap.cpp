/* NX_Cubemap.cpp -- API definition for Nexium's cubemap module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./NX_Cubemap.hpp"

#include "./INX_GPUProgramCache.hpp"
#include "./INX_RenderUtils.hpp"
#include "./INX_GlobalPool.hpp"
#include "./INX_GPUBridge.hpp"

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

void INX_EnsureCubemapFramebuffer(NX_Cubemap* cubemap, bool depth)
{
    if (depth && !cubemap->depth.IsValid())
    {
        cubemap->depth = gpu::Texture(
            gpu::TextureConfig
            {
                .target = GL_TEXTURE_2D,
                .internalFormat = GL_DEPTH_COMPONENT24,
                .data = nullptr,
                .width = cubemap->gpu.GetWidth(),
                .height = cubemap->gpu.GetHeight(),
                .immutable = true
            }
        );

        cubemap->framebuffer = gpu::Framebuffer(
            {&cubemap->gpu}, &cubemap->depth
        );
    }
    else if (!cubemap->framebuffer.IsValid()) {
        cubemap->framebuffer = gpu::Framebuffer(
            {&cubemap->gpu}, nullptr
        );
    }
}

// ============================================================================
// LOCAL FUNCTIONS
// ============================================================================

static gpu::Texture INX_LoadEquirectangular(const NX_Image& image)
{
    /* --- Determines the internal source and destination formats --- */

    GLenum srcInternalFormat = INX_GPU_GetInternalFormat(image.format, false);
    GLenum dstInternalFormat = INX_GPU_GetInternalFormat(image.format, true);

    /* --- Allocate cubemap texture --- */

    gpu::Texture texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = dstInternalFormat,
            .data = nullptr,
            .width = image.h,
            .height = image.h,
            .mipmap = true,
            .immutable = true
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
            .height = image.h,
            .immutable = true
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

    gpu::Framebuffer fb({&texture});

    gpu::Pipeline pipeline;

    pipeline.BindFramebuffer(fb);
    pipeline.SetViewport(fb);

    pipeline.BindTexture(0, panorama);
    pipeline.UseProgram(INX_Programs.GetCubemapFromEquirectangular());

    for (int i = 0; i < 6; i++) {
        fb.SetColorAttachmentTarget(0, 0, i);
        pipeline.SetUniformInt1(0, i);
        pipeline.Draw(GL_TRIANGLES, 3);
    }

    return texture;
}

static gpu::Texture INX_LoadLineHorizontal(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 6;

    /* --- Allocate cubemap texture --- */

    gpu::Texture texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = INX_GPU_GetInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize,
            .mipmap = true,
            .immutable = true
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
            uint8_t* dstRow = faceBuffer.GetData() + y * cubeFaceSize * bytesPerPixel;
            SDL_memcpy(dstRow, srcRow, cubeFaceSize * bytesPerPixel);
        }
        region.cubeFace = gpu::CubeFace(int(gpu::CubeFace::PositiveX) + i);
        texture.Upload(faceBuffer.GetData(), region);
    }

    return texture;
}

static gpu::Texture INX_LoadLineVertical(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.h / 6;

    /* --- Allocate cubemap texture --- */

    gpu::Texture texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = INX_GPU_GetInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize,
            .mipmap = true,
            .immutable = true
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
        texture.Upload(pixels + (i * cubeFaceSize * image.w * bytesPerPixel), region);
    }

    return texture;
}

static gpu::Texture INX_LoadCrossThreeByFour(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 3;

    /* --- Allocate cubemap texture --- */

    gpu::Texture texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = INX_GPU_GetInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize,
            .mipmap = true,
            .immutable = true
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
                uint8_t* dstRow = faceBuffer.GetData() + y * cubeFaceSize * bytesPerPixel;
                SDL_memcpy(dstRow, srcRow, cubeFaceSize * bytesPerPixel);
            }
            texture.Upload(faceBuffer.GetData(), gpu::UploadRegion{
                .x = 0, .y = 0, .z = 0,
                .width = cubeFaceSize,
                .height = cubeFaceSize,
                .depth = 1,
                .level = 0,
                .cubeFace = pos.face
            });
        }
    }

    return texture;
}

static gpu::Texture INX_LoadCrossFourByThree(const NX_Image& image)
{
    /* --- Calculate cube face size --- */

    int cubeFaceSize = image.w / 4;

    /* --- Allocate cubemap texture --- */

    gpu::Texture texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = INX_GPU_GetInternalFormat(image.format, false),
            .data = nullptr,
            .width = cubeFaceSize,
            .height = cubeFaceSize,
            .mipmap = true,
            .immutable = true
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
                uint8_t* dstRow = faceBuffer.GetData() + y * cubeFaceSize * bytesPerPixel;
                SDL_memcpy(dstRow, srcRow, cubeFaceSize * bytesPerPixel);
            }
            texture.Upload(faceBuffer.GetData(), gpu::UploadRegion{
                .x = 0, .y = 0, .z = 0,
                .width = cubeFaceSize,
                .height = cubeFaceSize,
                .depth = 1,
                .level = 0,
                .cubeFace = pos.face
            });
        }
    }

    return texture;
}

// ============================================================================
// PUBLIC API
// ============================================================================

NX_Cubemap* NX_CreateCubemap(int size, NX_PixelFormat format)
{
    NX_Cubemap* cubemap = INX_Pool.Create<NX_Cubemap>();

    cubemap->gpu = gpu::Texture(
        gpu::TextureConfig
        {
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = INX_GPU_GetInternalFormat(format, true),
            .data = nullptr,
            .width = size,
            .height = size,
            .depth = 0,
            .mipmap = true,
            .immutable = true
        },
        gpu::TextureParam
        {
            .minFilter = GL_LINEAR_MIPMAP_LINEAR,
            .magFilter = GL_LINEAR,
            .sWrap = GL_CLAMP_TO_EDGE,
            .tWrap = GL_CLAMP_TO_EDGE,
            .rWrap = GL_CLAMP_TO_EDGE,
            .anisotropy = 1.0f
        }
    );

    return cubemap;
}

NX_Cubemap* NX_LoadCubemapFromData(const NX_Image* image)
{
    NX_Cubemap* cubemap = INX_Pool.Create<NX_Cubemap>();

    /* --- Layout detection and cubemap loading --- */

    if (image->w > image->h) {
        if (image->w == 2 * image->h) {
            cubemap->gpu = INX_LoadEquirectangular(*image);
        }
        else if (image->w / 6 == image->h) {
            cubemap->gpu = INX_LoadLineHorizontal(*image);
        }
        else if (image->w / 4 == image->h / 3) {
            cubemap->gpu = INX_LoadCrossFourByThree(*image);
        }
    }
    else if (image->h > image->w) {
        if (image->h / 6 == image->w) {
            cubemap->gpu = INX_LoadLineVertical(*image);
        }
        else if (image->w / 3 == image->h / 4) {
            cubemap->gpu = INX_LoadCrossThreeByFour(*image);
        }
    }

    if (!cubemap->gpu.IsValid()) {
        NX_LOG(E, "RENDER: Unable to determine skybox cubemap layout");
        INX_Pool.Destroy(cubemap);
        return nullptr;
    }

    /* --- Generate mipmaps and setup parameters --- */

    cubemap->gpu.GenerateMipmap(); //< Needed for prefilter
    cubemap->gpu.SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    return cubemap;
}

NX_Cubemap* NX_LoadCubemap(const char* filePath)
{
    NX_Image image = NX_LoadImage(filePath);
    if (image.pixels == nullptr) return nullptr;

    NX_Cubemap* cubemap = NX_LoadCubemapFromData(&image);
    NX_DestroyImage(&image);

    return cubemap;
}

void NX_DestroyCubemap(NX_Cubemap* cubemap)
{
    INX_Pool.Destroy(cubemap);
}

void NX_GenerateSkybox(NX_Cubemap* cubemap, const NX_Skybox* skybox)
{
    INX_EnsureCubemapFramebuffer(cubemap, false);

    gpu::Pipeline pipeline;

    pipeline.BindFramebuffer(cubemap->framebuffer);
    pipeline.SetViewport(cubemap->framebuffer);

    pipeline.UseProgram(INX_Programs.GetCubemapSkybox());

    pipeline.SetUniformFloat3(1, NX_Vec3Normalize(-skybox->sunDirection));
    pipeline.SetUniformFloat3(2, skybox->skyColorTop);
    pipeline.SetUniformFloat3(3, skybox->skyColorHorizon);
    pipeline.SetUniformFloat3(4, skybox->sunColor);
    pipeline.SetUniformFloat3(5, skybox->groundColor);
    pipeline.SetUniformFloat1(6, skybox->sunSize);
    pipeline.SetUniformFloat1(7, skybox->haze);
    pipeline.SetUniformFloat1(8, skybox->energy);
    pipeline.SetUniformInt1(9, cubemap->gpu.IsHDR());

    for (int i = 0; i < 6; i++) {
        cubemap->framebuffer.SetColorAttachmentTarget(0, 0, i);
        pipeline.SetUniformMat4(0, INX_GetCubeView(i) * INX_GetCubeProj());
        pipeline.Draw(GL_TRIANGLES, 36);
    }

    cubemap->gpu.GenerateMipmap();
}
