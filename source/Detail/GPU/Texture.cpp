/* Texture.cpp -- Allows high-level GPU texture management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include "./Texture.hpp"
#include "./Pipeline.hpp"

namespace gpu {

/* === Public Implementation === */

void Texture::Realloc(const TextureConfig& config) noexcept
{
    if (!IsValid()) {
        NX_LOG(E, "GPU: Cannot replace invalid texture");
        return;
    }

    SDL_assert(config.target == mTarget && "realloc cannot change texture target"); // NOLINT
    config.Check();

    Pipeline::WithTextureBind(mTarget, mID,
        [&]()
        {
            AllocateTexture(config);

            if (config.data != nullptr) {
                if (mTarget == GL_TEXTURE_CUBE_MAP) {
                    const void* const* cubeData = static_cast<const void* const*>(config.data);
                    UploadCube(cubeData, 0);
                }
                else {
                    UploadRegion fullRegion;
                    fullRegion.level = 0;
                    UploadData_Bound(config.data, fullRegion);
                }
            }

            if (config.mipmap) {
                GenerateMipmap_Bound();
            }
        }
    );
}

void Texture::Upload(const void* data, int depth, int level) noexcept
{
    SDL_assert(IsValid() && "Cannot upload data to invalid texture"); // NOLINT

    UploadRegion region;
    region.depth = depth;
    region.level = level;

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        UploadData_Bound(data, region);
    });
}

void Texture::Upload(const void* data, const UploadRegion& region) noexcept
{
    SDL_assert(IsValid() && "Cannot upload data to invalid texture"); // NOLINT

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        UploadData_Bound(data, region);
    });
}

void Texture::UploadCube(const void* const* data, int level) noexcept
{
    SDL_assert(IsValid() && "Cannot upload cube data to invalid texture"); // NOLINT
    SDL_assert(mTarget == GL_TEXTURE_CUBE_MAP);

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        GLenum format, type;
        GetFormatAndType(mInternalFormat, format, type);
        for (int i = 0; i < 6; ++i) {
            const void* faceData = data ? data[i] : nullptr;
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, level, 0, 0, mWidth, mHeight, format, type, faceData);
        }
    });
}

void Texture::SetMipLevelRange(int baseLevel, int maxLevel) noexcept
{
    SDL_assert(IsValid() && "Cannot set sampling levels on invalid texture"); // NOLINT

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        SetMipLevelRange_Bound(baseLevel, maxLevel);
    });
}

void Texture::SetParameters(const TextureParam& parameters) noexcept
{
    SDL_assert(IsValid() && "Cannot set parameters on invalid texture"); // NOLINT

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        SetFilter_Bound(parameters.minFilter, parameters.magFilter);
        SetWrap_Bound(parameters.sWrap, parameters.tWrap, parameters.rWrap);
        SetAnisotropy_Bound(parameters.anisotropy);
    });
}

void Texture::SetWrap(GLenum sWrap, GLenum tWrap, GLenum rWrap) noexcept
{
    SDL_assert(IsValid() && "Cannot set wrap on invalid texture"); // NOLINT

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        SetWrap_Bound(sWrap, tWrap, rWrap);
    });
}

void Texture::SetFilter(GLenum minFilter, GLenum magFilter) noexcept
{
    SDL_assert(IsValid() && "Cannot set filter on invalid texture"); // NOLINT

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        SetFilter_Bound(minFilter, magFilter);
    });
}

void Texture::SetAnisotropy(float anisotropy) noexcept
{
    SDL_assert(IsValid() && "Cannot set anisotropy on invalid texture"); // NOLINT

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        SetAnisotropy_Bound(anisotropy);
    });
}

void Texture::GenerateMipmap() noexcept
{
    SDL_assert(IsValid() && "Cannot generate mipmap on invalid texture"); // NOLINT

    Pipeline::WithTextureBind(mTarget, mID, [&]() {
        GenerateMipmap_Bound();
    });
}

/* === Private Implementation === */

void Texture::CreateTexture(const TextureConfig& config, const TextureParam& param) noexcept
{
    mTarget = config.target;

    glGenTextures(1, &mID);
    if (mID == 0) {
        NX_LOG(E, "GPU: Failed to create texture object");
        return;
    }

    Pipeline::WithTextureBind(mTarget, mID,
        [&]()
        {
            AllocateTexture(config);
            if (!IsValid()) {
                return;
            }

            if (config.data != nullptr) {
                if (mTarget == GL_TEXTURE_CUBE_MAP) {
                    UploadCube(static_cast<const void* const*>(config.data), 0);
                }
                else {
                    UploadRegion fullRegion;
                    fullRegion.level = 0;
                    UploadData_Bound(config.data, fullRegion);
                }
            }

            if (config.mipmap) {
                GenerateMipmap_Bound();
            }

            SetFilter_Bound(param.minFilter, param.magFilter);
            SetWrap_Bound(param.sWrap, param.tWrap, param.rWrap);
            SetAnisotropy_Bound(param.anisotropy);
        }
    );
}

void Texture::AllocateTexture(const TextureConfig& config) noexcept
{
    FormatKey key = {config.target, config.internalFormat};

    /* --- Check cache --- */

    auto it = sFormatFallbacks.find(key);
    if (it != sFormatFallbacks.end()) {
        // Format already tested
        GLenum formatToUse = (it->second == GL_NONE) ? config.internalFormat : it->second;
        mInternalFormat = formatToUse;
        mWidth = config.width;
        mHeight = config.height;
        mDepth = config.depth;
        mMipLevels = 1;
        // Allocate directly with known format
        AllocateWithFormat(formatToUse);
        return;
    }

    /* --- Test the requested format and its fallbacks --- */

    GLenum currentFormat = config.internalFormat;
    while (currentFormat != GL_NONE)
    {
        mInternalFormat = currentFormat;
        mWidth = config.width;
        mHeight = config.height;
        mDepth = config.depth;
        mMipLevels = 1;

        // Try the allocation
        glGetError(); // Clean up previous errors
        if (AllocateWithFormat(currentFormat)) {
            if (currentFormat != config.internalFormat) {
                NX_LOG(W, "GPU: Format %s not supported for %s, using fallback %s",
                    FormatToString(config.internalFormat), TargetToString(config.target), FormatToString(currentFormat));
                sFormatFallbacks[key] = currentFormat;
            }
            else {
                sFormatFallbacks[key] = GL_NONE; // Original format supported
            }
            return;
        }

        // Failed, try fallback
        GLenum nextFormat = GetFallbackFormat(currentFormat);
        if (nextFormat == currentFormat) {
            break; // No more fallbacks
        }
        currentFormat = nextFormat;
    }

    /* --- All formats failed --- */

    NX_LOG(E, "GPU: All formats failed for %s (%dx%dx%d), texture creation failed",
        TargetToString(config.target), config.width, config.height, config.depth);

    sFormatFallbacks[key] = GL_NONE;    // Mark as tested but failed
    DestroyTexture();                   // Invalidate texture
}

bool Texture::AllocateWithFormat(GLenum internalFormat) noexcept
{
    GLenum format, type;
    GetFormatAndType(internalFormat, format, type);

    switch (mTarget) {
    case GL_TEXTURE_2D:
        glTexImage2D(mTarget, 0, internalFormat, mWidth, mHeight, 0, format, type, nullptr);
        break;
    case GL_TEXTURE_3D:
    case GL_TEXTURE_2D_ARRAY:
        glTexImage3D(mTarget, 0, internalFormat, mWidth, mHeight, mDepth, 0, format, type, nullptr);
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        glTexImage3D(mTarget, 0, internalFormat, mWidth, mHeight, mDepth * 6, 0, format, type, nullptr);
        break;
    case GL_TEXTURE_CUBE_MAP:
        {
            const GLenum faces[6] = {
                GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
            };

            for (int i = 0; i < 6; ++i) {
                glTexImage2D(faces[i], 0, internalFormat, mWidth, mHeight, 0, format, type, nullptr);
                if (glGetError() != GL_NO_ERROR) {
                    return false;
                }
            }
        }
        break;
    default:
        SDL_assert(false && "Unsupported texture target"); // NOLINT
        return false;
    }

    return glGetError() == GL_NO_ERROR;
}

void Texture::UploadData_Bound(const void* data, const UploadRegion& region) noexcept
{
    GLenum format, type;
    GetFormatAndType(mInternalFormat, format, type);

    // Calculate dimensions (0 = full texture)
    int uploadWidth = (region.width > 0) ? region.width : mWidth;
    int uploadHeight = (region.height > 0) ? region.height : mHeight;
    int uploadDepth = (region.depth > 0) ? region.depth : mDepth;

    switch (mTarget) {
    case GL_TEXTURE_2D:
        glTexSubImage2D(mTarget, region.level, region.x, region.y, uploadWidth, uploadHeight, format, type, data);
        break;
    case GL_TEXTURE_3D:
    case GL_TEXTURE_2D_ARRAY:
        glTexSubImage3D(mTarget, region.level, region.x, region.y, region.z, uploadWidth, uploadHeight, uploadDepth, format, type, data);
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        glTexSubImage3D(mTarget, region.level, region.x, region.y, region.z, uploadWidth, uploadHeight, uploadDepth * 6, format, type, data);
        break;
    case GL_TEXTURE_CUBE_MAP:
        {
            GLenum face = static_cast<GLenum>(region.cubeFace);
            glTexSubImage2D(face, region.level, region.x, region.y, uploadWidth, uploadHeight, format, type, data);
        }
        break;
    default:
        SDL_assert(false && "Unsupported texture target"); // NOLINT
        break;
    }
}

void Texture::SetWrap_Bound(GLenum sWrap, GLenum tWrap, GLenum rWrap) noexcept
{
    glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, tWrap);
    if (mTarget == GL_TEXTURE_3D || mTarget == GL_TEXTURE_CUBE_MAP ||
        mTarget == GL_TEXTURE_2D_ARRAY || mTarget == GL_TEXTURE_CUBE_MAP_ARRAY) {
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, rWrap);
    }
}

void Texture::SetFilter_Bound(GLenum minFilter, GLenum magFilter) noexcept
{
    glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
}

void Texture::SetMipLevelRange_Bound(int baseLevel, int maxLevel) noexcept
{
    glTexParameteri(mTarget, GL_TEXTURE_BASE_LEVEL, baseLevel);
    glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

void Texture::SetAnisotropy_Bound(float anisotropy) noexcept
{
    /* --- Check anisotropy support --- */

    if (!sAnisotropyInitialized)
    {
        if (GLAD_GL_EXT_texture_filter_anisotropic) {
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &sMaxAnisotropy);
            NX_LOG(D, "GPU: Anisotropic filtering supported, max anisotropy: %.1f", sMaxAnisotropy);
        }
        else {
            sMaxAnisotropy = 1.0f;
            NX_LOG(D, "GPU: Anisotropic filtering not supported");
        }

        sAnisotropyInitialized = true;
    }

    /* --- Set anisotropy if supported --- */

    if (GLAD_GL_EXT_texture_filter_anisotropic) {
        glTexParameterf(mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, NX_MIN(anisotropy, sMaxAnisotropy));
    }
}

void Texture::GenerateMipmap_Bound() noexcept
{
    glGenerateMipmap(mTarget);

    mMipLevels = CalculateMaxMipLevels(
        mWidth, mHeight, mDepth
    );
}

} // namespace gpu
