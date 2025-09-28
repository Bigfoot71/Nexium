/* Texture.hpp -- Allows high-level GPU texture management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_GPU_TEXTURE_HPP
#define HP_GPU_TEXTURE_HPP

#include <Hyperion/HP_Core.h>
#include <SDL3/SDL_assert.h>
#include <glad/gles2.h>

#include <unordered_map>
#include <utility>

namespace gpu {

/* === Forward Declaration === */

class Pipeline;
class Framebuffer;

/* === TextureConfig === */

struct TextureParam {
    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;
    GLenum sWrap = GL_CLAMP_TO_EDGE;
    GLenum tWrap = GL_CLAMP_TO_EDGE;
    GLenum rWrap = GL_CLAMP_TO_EDGE;
    float anisotropy = 1.0f;
};

struct TextureConfig {
    GLenum target = GL_TEXTURE_2D;
    GLenum internalFormat = GL_RGBA8;
    const void* data = nullptr;
    int width = 0;
    int height = 0;
    int depth = 0;
    bool mipmap = false;

    const TextureConfig& check() const {
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
            SDL_assert(false && "Unsupported texture target");
            break;
        }
        return *this;
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
    bool isValid() const noexcept;
    bool isHDR() const noexcept;
    GLuint id() const noexcept;
    GLenum target() const noexcept;
    GLenum internalFormat() const noexcept;
    bool hasMipmap() const noexcept;
    int numLevels() const noexcept;
    HP_IVec2 dimensions() const noexcept;
    int width() const noexcept;
    int height() const noexcept;
    int depth() const noexcept;

    /** Post-creation manipulation (keeps the ID, only affects the current target) */
    void realloc(int w, int h, int d, const void* data = nullptr) noexcept;
    void realloc(const TextureConfig& config) noexcept;

    /** Ensures the required dimensions are available, calls realloc if the size needs to be increased */
    void reserve(int w, int h, int d) noexcept;

    /** Data upload */
    void upload(const void* data, int depth = 0, int level = 0) noexcept;
    void upload(const void* data, const UploadRegion& region) noexcept;
    void uploadCube(const void* const* data, int level = 0) noexcept;

    /** Texture parameters */
    void setMipLevelRange(int baseLevel, int maxLevel) noexcept;
    void setParameters(const TextureParam& parameters) noexcept;
    void setWrap(GLenum sWrap, GLenum tWrap, GLenum rWrap = GL_CLAMP_TO_EDGE) noexcept;
    void setFilter(GLenum minFilter, GLenum magFilter) noexcept;
    void setAnisotropy(float anisotropy) noexcept;
    void generateMipmap() noexcept;

private:
    /** Member variables */
    GLuint mID{0};
    GLenum mTarget{GL_TEXTURE_2D};
    GLenum mInternalFormat{GL_RGBA8};
    int mWidth{0}, mHeight{0}, mDepth{0};
    int mMipLevels{1};

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
    void createTexture(const TextureConfig& config, const TextureParam& param) noexcept;
    void allocateTexture(const TextureConfig& config) noexcept; // Allocates via glTexImage*, tests fallbacks
    bool allocateWithFormat(GLenum internalFormat) noexcept;    // Attempts allocation with a specific format
    void destroyTexture() noexcept;

    /** Internal operations (require the texture to be bound) */
    void uploadData_Bound(const void* data, const UploadRegion& region) noexcept;
    void setWrap_Bound(GLenum sWrap, GLenum tWrap, GLenum rWrap) noexcept;
    void setFilter_Bound(GLenum minFilter, GLenum magFilter) noexcept;
    void setMipLevelRange_Bound(int baseLevel, int maxLevel) noexcept;
    void setAnisotropy_Bound(float anisotropy) noexcept;
    void generateMipmap_Bound() noexcept;

    /** Static format helpers */
    static GLenum getFormatAndType(GLenum internalFormat, GLenum& format, GLenum& type) noexcept;
    static GLenum getFallbackFormat(GLenum internalFormat) noexcept;
    static const char* formatToString(GLenum internalFormat) noexcept;
    static const char* targetToString(GLenum target) noexcept;

    /** Static mipmap helpers */
    static int calculateMaxMipLevels(int width, int height, int depth = 1) noexcept;
};

/* === Inline Implementations === */

inline Texture::Texture(const TextureConfig& config, const TextureParam& param) noexcept
{
    createTexture(config.check(), param);
}

inline Texture::~Texture() noexcept
{
    destroyTexture();
}

inline Texture::Texture(Texture&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mTarget(other.mTarget)
    , mInternalFormat(other.mInternalFormat)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mDepth(other.mDepth)
    , mMipLevels(other.mMipLevels)
{ }

inline Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        destroyTexture();
        mID = std::exchange(other.mID, 0);
        mTarget = other.mTarget;
        mInternalFormat = other.mInternalFormat;
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mDepth = other.mDepth;
        mMipLevels = other.mMipLevels;
    }
    return *this;
}

inline bool Texture::isValid() const noexcept
{
    return mID > 0;
}

inline bool Texture::isHDR() const noexcept
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

inline GLuint Texture::id() const noexcept
{
    return mID;
}

inline GLenum Texture::target() const noexcept
{
    return mTarget;
}

inline GLenum Texture::internalFormat() const noexcept
{
    return mInternalFormat;
}

inline bool Texture::hasMipmap() const noexcept
{
    return mMipLevels > 1;
}

inline int Texture::numLevels() const noexcept
{
    return mMipLevels;
}

inline HP_IVec2 Texture::dimensions() const noexcept
{
    return HP_IVEC2(mWidth, mHeight);
}

inline int Texture::width() const noexcept
{
    return mWidth;
}

inline int Texture::height() const noexcept
{
    return mHeight;
}

inline int Texture::depth() const noexcept
{
    return mDepth;
}

inline void Texture::realloc(int w, int h, int d, const void* data) noexcept
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

    realloc(config);
}

inline void Texture::reserve(int w, int h, int d) noexcept
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

    realloc(config);
}

inline void Texture::destroyTexture() noexcept
{
    if (mID != 0) {
        glDeleteTextures(1, &mID);
        mID = 0;
    }
}

/* === Format Helpers === */

inline GLenum Texture::getFormatAndType(GLenum internalFormat, GLenum& format, GLenum& type) noexcept
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

inline GLenum Texture::getFallbackFormat(GLenum internalFormat) noexcept
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

inline const char* Texture::formatToString(GLenum internalFormat) noexcept
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

inline const char* Texture::targetToString(GLenum target) noexcept
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

inline int Texture::calculateMaxMipLevels(int width, int height, int depth) noexcept
{
    int maxDimension = HP_MAX3(width, height, depth);
    return static_cast<int>(std::floor(std::log2(maxDimension))) + 1;
}

} // namespace gpu

#endif // HP_GPU_TEXTURE_HPP
