/* Texture.hpp -- Allows high-level GPU texture management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_TEXTURE_HPP
#define NX_GPU_TEXTURE_HPP

#include <NX/NX_Math.h>

#include <SDL3/SDL_assert.h>
#include <glad/gles2.h>

#include <unordered_map>
#include <utility>

namespace gpu {

/* === Forward Declaration === */

class Pipeline;
class Framebuffer;

/* === TextureConfig === */

struct TextureConfig {
    GLenum target = GL_TEXTURE_2D;
    GLenum internalFormat = GL_RGBA8;
    const void* data = nullptr;
    int width = 0;
    int height = 0;
    int depth = 0;
    bool mipmap = false;

    const TextureConfig& Check() const {
        switch (target) {
        case GL_TEXTURE_2D:
            SDL_assert(width > 0 && height > 0);
            break;
        case GL_TEXTURE_3D:
            SDL_assert(width > 0 && height > 0 && depth > 0);
            break;
        case GL_TEXTURE_2D_ARRAY:
            SDL_assert(width > 0 && height > 0);
            SDL_assert(depth > 0);
            break;
        case GL_TEXTURE_CUBE_MAP:
            SDL_assert(width > 0 && width == height);
            break;
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            SDL_assert(width > 0 && width == height);
            SDL_assert(depth > 0);
            break;
        default:
            SDL_assert(false && "Unsupported texture target"); // NOLINT
            break;
        }
        return *this;
    }
};

struct TextureParam {
    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;
    GLenum sWrap = GL_CLAMP_TO_EDGE;
    GLenum tWrap = GL_CLAMP_TO_EDGE;
    GLenum rWrap = GL_CLAMP_TO_EDGE;
    float anisotropy = 1.0f;

    bool operator==(const TextureParam& other) const {
        return minFilter == other.minFilter &&
               magFilter == other.magFilter &&
               sWrap == other.sWrap &&
               tWrap == other.tWrap &&
               rWrap == other.rWrap &&
               anisotropy == other.anisotropy;
    }

    bool operator!=(const TextureParam& other) const {
        return !(*this == other);
    }
};

/* === Upload Structure === */

enum class CubeFace : GLenum {
    PositiveX = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    NegativeX = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    PositiveY = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    NegativeY = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    PositiveZ = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    NegativeZ = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

struct UploadRegion {
    int x = 0;
    int y = 0;
    int z = 0;
    int width = 0;
    int height = 0;
    int depth = 1;
    int level = 0;
    CubeFace cubeFace = CubeFace::PositiveX;
};

/* === Declaration === */

class Texture {
public:
    /** Constructors */
    Texture() = default;

    /** Generic constructor via configuration */
    Texture(const TextureConfig& config, const TextureParam& param = {}) noexcept;

    /** Destructor and move semantics */
    ~Texture() noexcept;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    /** Public interface */
    bool IsValid() const noexcept;
    bool IsHDR() const noexcept;
    GLuint GetID() const noexcept;
    GLenum GetTarget() const noexcept;
    GLenum GetInternalFormat() const noexcept;
    bool HasMipmap() const noexcept;
    int GetNumLevels() const noexcept;
    NX_IVec2 GetDimensions() const noexcept;
    int GetWidth() const noexcept;
    int GetHeight() const noexcept;
    int GetDepth() const noexcept;
    const TextureParam& GetParameters() const noexcept;

    /** Post-creation manipulation (keeps the ID, only affects the current target) */
    void Realloc(int w, int h, int d, const void* data = nullptr) noexcept;
    void Realloc(const TextureConfig& config) noexcept;

    /** Ensures the required dimensions are available, calls realloc if the size needs to be increased */
    void Reserve(int w, int h, int d) noexcept;

    /** Only valid for array textures; the texture will have a new ID if keepData is true */
    void ReallocLayers(int count, bool keepData = false) noexcept;
    void ReserveLayers(int count, bool keepData = false) noexcept;

    /** Data upload */
    void Upload(const void* data, int depth = 0, int level = 0) noexcept;
    void Upload(const void* data, const UploadRegion& region) noexcept;
    void UploadCube(const void* const* data, int level = 0) noexcept;

    /** Texture parameters */
    void SetMipLevelRange(int baseLevel, int maxLevel) noexcept;
    void SetParameters(const TextureParam& parameters) noexcept;
    void SetWrap(GLenum sWrap, GLenum tWrap, GLenum rWrap = GL_CLAMP_TO_EDGE) noexcept;
    void SetFilter(GLenum minFilter, GLenum magFilter) noexcept;
    void SetAnisotropy(float anisotropy) noexcept;
    void GenerateMipmap() noexcept;

private:
    /** Member variables */
    GLuint mID{0};
    GLenum mTarget{GL_TEXTURE_2D};
    GLenum mInternalFormat{GL_RGBA8};
    int mWidth{0}, mHeight{0}, mDepth{0};
    int mMipLevels{1};
    TextureParam mParameters{};

    /** Anisotropy support */
    static inline bool sAnisotropyInitialized = false;
    static inline float sMaxAnisotropy = 1.0f;

    /** Fallback format cache by target */
    using FormatKey = std::pair<GLenum, GLenum>; // {target, internalFormat}
    struct FormatKeyHash {
        std::size_t operator()(const FormatKey& key) const {
            return std::hash<GLenum>{}(key.first) ^ (std::hash<GLenum>{}(key.second) << 1);
        }
    };
    static inline std::unordered_map<FormatKey, GLenum, FormatKeyHash> sFormatFallbacks;

    /** Creation and allocation */
    void CreateTexture(const TextureConfig& config, const TextureParam& param) noexcept;
    void AllocateTexture(const TextureConfig& config) noexcept; // Allocates via glTexImage*, tests fallbacks
    bool AllocateWithFormat(GLenum internalFormat) noexcept;    // Attempts allocation with a specific format
    void DestroyTexture() noexcept;

    /** Internal operations (require the texture to be bound) */
    void UploadData_Bound(const void* data, const UploadRegion& region) noexcept;
    void SetWrap_Bound(GLenum sWrap, GLenum tWrap, GLenum rWrap) noexcept;
    void SetFilter_Bound(GLenum minFilter, GLenum magFilter) noexcept;
    void SetMipLevelRange_Bound(int baseLevel, int maxLevel) noexcept;
    void SetAnisotropy_Bound(float anisotropy) noexcept;
    void GenerateMipmap_Bound() noexcept;

    /** Static format helpers */
    static GLenum GetFormatAndType(GLenum internalFormat, GLenum& format, GLenum& type) noexcept;
    static GLenum GetFallbackFormat(GLenum internalFormat) noexcept;
    static const char* FormatToString(GLenum internalFormat) noexcept;
    static const char* TargetToString(GLenum target) noexcept;

    /** Static mipmap helpers */
    static int CalculateMaxMipLevels(int width, int height, int depth = 1) noexcept;
};

/* === Inline Implementations === */

inline Texture::Texture(const TextureConfig& config, const TextureParam& param) noexcept
    : mParameters(param)
{
    CreateTexture(config.Check(), param);
}

inline Texture::~Texture() noexcept
{
    DestroyTexture();
}

inline Texture::Texture(Texture&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mTarget(other.mTarget)
    , mInternalFormat(other.mInternalFormat)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mDepth(other.mDepth)
    , mMipLevels(other.mMipLevels)
    , mParameters(other.mParameters)
{ }

inline Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        DestroyTexture();
        mID = std::exchange(other.mID, 0);
        mTarget = other.mTarget;
        mInternalFormat = other.mInternalFormat;
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mDepth = other.mDepth;
        mMipLevels = other.mMipLevels;
        mParameters = other.mParameters;
    }
    return *this;
}

inline bool Texture::IsValid() const noexcept
{
    return mID > 0;
}

inline bool Texture::IsHDR() const noexcept
{
    switch (mInternalFormat) {
    case GL_R16F:
    case GL_RG16F:
    case GL_RGB16F:
    case GL_RGBA16F:
    case GL_R32F:
    case GL_RG32F:
    case GL_RGB32F:
    case GL_RGBA32F:
        return true;
    default:
        break;
    }
    return false;
}

inline GLuint Texture::GetID() const noexcept
{
    return mID;
}

inline GLenum Texture::GetTarget() const noexcept
{
    return mTarget;
}

inline GLenum Texture::GetInternalFormat() const noexcept
{
    return mInternalFormat;
}

inline bool Texture::HasMipmap() const noexcept
{
    return mMipLevels > 1;
}

inline int Texture::GetNumLevels() const noexcept
{
    return mMipLevels;
}

inline NX_IVec2 Texture::GetDimensions() const noexcept
{
    return NX_IVEC2(mWidth, mHeight);
}

inline int Texture::GetWidth() const noexcept
{
    return mWidth;
}

inline int Texture::GetHeight() const noexcept
{
    return mHeight;
}

inline int Texture::GetDepth() const noexcept
{
    return mDepth;
}

inline const TextureParam& Texture::GetParameters() const noexcept
{
    return mParameters;
}

inline void Texture::Realloc(int w, int h, int d, const void* data) noexcept
{
    TextureConfig config {
        .target = mTarget,
        .internalFormat = mInternalFormat,
        .data = data,
        .width = w,
        .height = h,
        .depth = d,
        .mipmap = (mMipLevels > 1)
    };

    Realloc(config);
}

inline void Texture::Reserve(int w, int h, int d) noexcept
{
    switch (mTarget) {
    case GL_TEXTURE_2D:
        if (w <= mWidth && h <= mHeight) {
            return;
        }
        break;
    case GL_TEXTURE_3D:
        if (w <= mWidth && h <= mHeight && d <= mDepth) {
            return;
        }
        break;
    case GL_TEXTURE_2D_ARRAY:
        if (w <= mWidth && h <= mHeight && d <= mDepth) {
            return;
        }
        break;
    case GL_TEXTURE_CUBE_MAP:
        if (w <= mWidth) {
            return;
        }
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        if (w <= mWidth && d <= mDepth) {
            return;
        }
        break;
    default:
        return;
    }

    TextureConfig config {
        .target = mTarget,
        .internalFormat = mInternalFormat,
        .data = nullptr,
        .width = w,
        .height = h,
        .depth = d,
        .mipmap = (mMipLevels > 1)
    };

    Realloc(config);
}

inline void Texture::ReserveLayers(int count, bool keepData) noexcept
{
    if (count > mDepth) {
        ReallocLayers(count, keepData);
    }
}

inline void Texture::DestroyTexture() noexcept
{
    if (mID != 0) {
        glDeleteTextures(1, &mID);
        mID = 0;
    }
}

/* === Format Helpers === */

inline GLenum Texture::GetFormatAndType(GLenum internalFormat, GLenum& format, GLenum& type) noexcept
{
    switch (internalFormat) {
    case GL_R8:           format = GL_RED;   type = GL_UNSIGNED_BYTE; break;
    case GL_RG8:          format = GL_RG;    type = GL_UNSIGNED_BYTE; break;
    case GL_RGB8:         format = GL_RGB;   type = GL_UNSIGNED_BYTE; break;
    case GL_RGBA8:        format = GL_RGBA;  type = GL_UNSIGNED_BYTE; break;
    case GL_R16F:         format = GL_RED;   type = GL_HALF_FLOAT; break;
    case GL_RG16F:        format = GL_RG;    type = GL_HALF_FLOAT; break;
    case GL_RGB16F:       format = GL_RGB;   type = GL_HALF_FLOAT; break;
    case GL_RGBA16F:      format = GL_RGBA;  type = GL_HALF_FLOAT; break;
    case GL_R32F:         format = GL_RED;   type = GL_FLOAT; break;
    case GL_RG32F:        format = GL_RG;    type = GL_FLOAT; break;
    case GL_RGB32F:       format = GL_RGB;   type = GL_FLOAT; break;
    case GL_RGBA32F:      format = GL_RGBA;  type = GL_FLOAT; break;
    case GL_R11F_G11F_B10F: format = GL_RGB; type = GL_UNSIGNED_INT_10F_11F_11F_REV; break;
    case GL_DEPTH_COMPONENT16:  format = GL_DEPTH_COMPONENT; type = GL_UNSIGNED_SHORT; break;
    case GL_DEPTH_COMPONENT24:  format = GL_DEPTH_COMPONENT; type = GL_UNSIGNED_INT; break;
    case GL_DEPTH_COMPONENT32F: format = GL_DEPTH_COMPONENT; type = GL_FLOAT; break;
    case GL_DEPTH24_STENCIL8:   format = GL_DEPTH_STENCIL; type = GL_UNSIGNED_INT_24_8; break;
    case GL_DEPTH32F_STENCIL8:  format = GL_DEPTH_STENCIL; type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV; break;
    default: format = GL_RGBA; type = GL_UNSIGNED_BYTE; break;
    }
    return internalFormat;
}

inline GLenum Texture::GetFallbackFormat(GLenum internalFormat) noexcept
{
    switch (internalFormat) {
    case GL_R11F_G11F_B10F:     return GL_RGB16F;
    case GL_R16F:               return GL_R8;
    case GL_RG16F:              return GL_RG8;
    case GL_RGB16F:             return GL_RGB8;
    case GL_RGBA16F:            return GL_RGBA8;
    case GL_R32F:               return GL_R16F;
    case GL_RG32F:              return GL_RG16F;
    case GL_RGB32F:             return GL_RGB16F;
    case GL_RGBA32F:            return GL_RGBA16F;
    case GL_DEPTH_COMPONENT32F: return GL_DEPTH_COMPONENT24;
    case GL_DEPTH_COMPONENT24:  return GL_DEPTH_COMPONENT16;
    case GL_DEPTH32F_STENCIL8:  return GL_DEPTH24_STENCIL8;
    default:                    return internalFormat;
    }
}

inline const char* Texture::FormatToString(GLenum internalFormat) noexcept
{
    switch (internalFormat) {
    case GL_R8: return "GL_R8";
    case GL_RG8: return "GL_RG8";
    case GL_RGB8: return "GL_RGB8";
    case GL_RGBA8: return "GL_RGBA8";
    case GL_R16F: return "GL_R16F";
    case GL_RG16F: return "GL_RG16F";
    case GL_RGB16F: return "GL_RGB16F";
    case GL_RGBA16F: return "GL_RGBA16F";
    case GL_R32F: return "GL_R32F";
    case GL_RG32F: return "GL_RG32F";
    case GL_RGB32F: return "GL_RGB32F";
    case GL_RGBA32F: return "GL_RGBA32F";
    case GL_R11F_G11F_B10F: return "GL_R11F_G11F_B10F";
    case GL_DEPTH_COMPONENT16: return "GL_DEPTH_COMPONENT16";
    case GL_DEPTH_COMPONENT24: return "GL_DEPTH_COMPONENT24";
    case GL_DEPTH_COMPONENT32F: return "GL_DEPTH_COMPONENT32F";
    case GL_DEPTH24_STENCIL8: return "GL_DEPTH24_STENCIL8";
    case GL_DEPTH32F_STENCIL8: return "GL_DEPTH32F_STENCIL8";
    default: return "Unknown";
    }
}

inline const char* Texture::TargetToString(GLenum target) noexcept
{
    switch (target) {
    case GL_TEXTURE_2D: return "GL_TEXTURE_2D";
    case GL_TEXTURE_3D: return "GL_TEXTURE_3D";
    case GL_TEXTURE_2D_ARRAY: return "GL_TEXTURE_2D_ARRAY";
    case GL_TEXTURE_CUBE_MAP: return "GL_TEXTURE_CUBE_MAP";
    case GL_TEXTURE_CUBE_MAP_ARRAY: return "GL_TEXTURE_CUBE_MAP_ARRAY";
    default: return "Unknown";
    }
}

inline int Texture::CalculateMaxMipLevels(int width, int height, int depth) noexcept
{
    int maxDimension = NX_MAX3(width, height, depth);
    return static_cast<int>(std::floor(std::log2(maxDimension))) + 1;
}

} // namespace gpu

#endif // NX_GPU_TEXTURE_HPP
