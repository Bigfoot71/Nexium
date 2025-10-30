/* Program.hpp -- Allows high-level GPU program management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_PROGRAM_HPP
#define NX_GPU_PROGRAM_HPP

#include <NX/NX_Math.h>
#include <NX/NX_Log.h>

#include "../Util/FixedArray.hpp"
#include "../BuildInfo.hpp"
#include "./Shader.hpp"

#include <SDL3/SDL_assert.h>
#include <glad/gles2.h>
#include <cstring>
#include <string>
#include <array>

namespace gpu {

/* === Declaration === */

class Program {
public:
    Program() = default;

    template<typename... Shaders>
    explicit Program(const Shaders&... shaders) noexcept;

    ~Program() noexcept;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    Program(Program&& other) noexcept;
    Program& operator=(Program&& other) noexcept;

    /** Returns '-1' on failure */
    int getUniformLocation(const char* name) const noexcept;

    /** Returns '-1' on failure */
    int getUniformBlockIndex(const char* name) const noexcept;

    /** Returns size of the uniform block */
    size_t getUniformBlockSize(int blockIndex) const noexcept;

    /** Set uniform binding point */
    void setUniformBlockBinding(int blockIndex, uint32_t blockBinding) noexcept;

    /** Simple getters */
    bool isValid() const noexcept;
    GLuint id() const noexcept;

private:
    friend class Pipeline;

private:
    void setUint1(int location, uint32_t value) const noexcept;
    void setUint2(int location, const NX_IVec2& value) const noexcept;
    void setUint3(int location, const NX_IVec3& value) const noexcept;
    void setUint4(int location, const NX_IVec4& value) const noexcept;
    void setInt1(int location, int value) const noexcept;
    void setInt2(int location, const NX_IVec2& value) const noexcept;
    void setInt3(int location, const NX_IVec3& value) const noexcept;
    void setInt4(int location, const NX_IVec4& value) const noexcept;
    void setFloat1(int location, float value) const noexcept;
    void setFloat2(int location, const NX_Vec2& value) const noexcept;
    void setFloat3(int location, const NX_Vec3& value) const noexcept;
    void setFloat3(int location, const NX_Color& value) const noexcept;
    void setFloat4(int location, const NX_Vec4& value) const noexcept;
    void setFloat4(int location, const NX_Quat& value) const noexcept;
    void setFloat4(int location, const NX_Color& value) const noexcept;
    void setMat3(int location, const NX_Mat3& value) const noexcept;
    void setMat3(int location, const NX_Mat4& value) const noexcept;
    void setMat4(int location, const NX_Mat4& value) const noexcept;

private:
    template<typename... Shaders>
    bool initProgram(const Shaders&... shaders) noexcept;
    bool linkProgram() noexcept;
    template<typename... Shaders>
    bool validateShaderStages(const Shaders&... shaders) noexcept;
    bool createUniformCache() noexcept;
    void cleanup() noexcept;

private:
    GLuint mID{0};

private:
    mutable util::FixedArray<std::array<uint8_t, 64>> mUniformCache{};
};

/* === Public Implementation === */

template<typename... Shaders>
inline Program::Program(const Shaders&... shaders) noexcept
{
    static_assert(sizeof...(Shaders) > 0, "At least one shader is required");
    
    if (!initProgram(shaders...)) {
        cleanup();
    }
}

inline Program::~Program() noexcept
{
    cleanup();
}

inline Program::Program(Program&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mUniformCache(std::move(other.mUniformCache))
{ }

inline Program& Program::operator=(Program&& other) noexcept
{
    if (this != &other) {
        cleanup();
        mID = std::exchange(other.mID, 0);
        mUniformCache = std::move(other.mUniformCache);
    }
    return *this;
}

inline int Program::getUniformLocation(const char* name) const noexcept
{
    return glGetUniformLocation(mID, name);
}

inline int Program::getUniformBlockIndex(const char* name) const noexcept
{
    GLuint blockIndex = glGetUniformBlockIndex(mID, name);
    if (blockIndex == GL_INVALID_INDEX) {
        glGetError(); // cleanup error
        return -1;
    }
    return static_cast<int>(blockIndex);
}

inline size_t Program::getUniformBlockSize(int blockIndex) const noexcept
{
    SDL_assert(blockIndex >= 0);

    GLint blockSize = 0;
    glGetActiveUniformBlockiv(mID, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    return static_cast<int>(blockSize);
}

inline void Program::setUniformBlockBinding(int blockIndex, uint32_t blockBinding) noexcept
{
    SDL_assert(blockIndex >= 0);

    glUniformBlockBinding(mID, blockIndex, blockBinding);
}

inline bool Program::isValid() const noexcept
{
    return (mID > 0);
}

inline GLuint Program::id() const noexcept
{
    return mID;
}

/* === Private Implementation === */

inline void Program::setUint1(int location, uint32_t value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform1ui(location, value);
    }
}

inline void Program::setUint2(int location, const NX_IVec2& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());
    SDL_assert(value.x > 0 && value.y > 0);

    uint32_t v[2] = {
        static_cast<uint32_t>(value.x),
        static_cast<uint32_t>(value.y)
    };

    if (std::memcmp(mUniformCache[location].data(), v, sizeof(v)) != 0) {
        std::memcpy(mUniformCache[location].data(), v, sizeof(v));
        glUniform2uiv(location, 1, v);
    }
}

inline void Program::setUint3(int location, const NX_IVec3& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());
    SDL_assert(value.x >= 0 && value.y >= 0 && value.z >= 0);

    uint32_t v[3] = {
        static_cast<uint32_t>(value.x),
        static_cast<uint32_t>(value.y),
        static_cast<uint32_t>(value.z)
    };

    if (std::memcmp(mUniformCache[location].data(), v, sizeof(v)) != 0) {
        std::memcpy(mUniformCache[location].data(), v, sizeof(v));
        glUniform3uiv(location, 1, v);
    }
}

inline void Program::setUint4(int location, const NX_IVec4& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());
    SDL_assert(value.x >= 0 && value.y >= 0 && value.z >= 0 && value.w >= 0);

    uint32_t v[4] = {
        static_cast<uint32_t>(value.x),
        static_cast<uint32_t>(value.y),
        static_cast<uint32_t>(value.z),
        static_cast<uint32_t>(value.w)
    };

    if (std::memcmp(mUniformCache[location].data(), v, sizeof(v)) != 0) {
        std::memcpy(mUniformCache[location].data(), v, sizeof(v));
        glUniform4uiv(location, 1, v);
    }
}

inline void Program::setInt1(int location, int value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform1i(location, value);
    }
}

inline void Program::setInt2(int location, const NX_IVec2& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform2iv(location, 1, reinterpret_cast<const int*>(&value));
    }
}

inline void Program::setInt3(int location, const NX_IVec3& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform3iv(location, 1, reinterpret_cast<const int*>(&value));
    }
}

inline void Program::setInt4(int location, const NX_IVec4& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform4iv(location, 1, reinterpret_cast<const int*>(&value));
    }
}

inline void Program::setFloat1(int location, float value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform1f(location, value);
    }
}

inline void Program::setFloat2(int location, const NX_Vec2& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform2fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat3(int location, const NX_Vec3& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform3fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat3(int location, const NX_Color& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(NX_Vec3));
        glUniform3fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat4(int location, const NX_Vec4& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform4fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat4(int location, const NX_Quat& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    NX_Vec4 quat = { value.x, value.y, value.z, value.w };
    if (std::memcmp(mUniformCache[location].data(), &quat, sizeof(quat)) != 0) {
        std::memcpy(mUniformCache[location].data(), &quat, sizeof(quat));
        glUniform4fv(location, 1, reinterpret_cast<const float*>(&quat));
    }
}

inline void Program::setFloat4(int location, const NX_Color& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(NX_Vec4));
        glUniform4fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setMat3(int location, const NX_Mat3& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), value.a, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), value.a, sizeof(value));
        glUniformMatrix3fv(location, 1, GL_FALSE, value.a);
    }
}

inline void Program::setMat3(int location, const NX_Mat4& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    union {
        float v[3][3];
        float a[9];
    } m33;

    for (int i = 0; i < 3; i++) {
        m33.v[i][0] = value.v[i][0];
        m33.v[i][1] = value.v[i][1];
        m33.v[i][2] = value.v[i][2];
    }

    if (std::memcmp(mUniformCache[location].data(), m33.a, sizeof(m33)) != 0) {
        std::memcpy(mUniformCache[location].data(), m33.a, sizeof(m33));
        glUniformMatrix3fv(location, 1, GL_FALSE, m33.a);
    }
}

inline void Program::setMat4(int location, const NX_Mat4& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const float*>(&value));
    }
}

template<typename... Shaders>
inline bool Program::initProgram(const Shaders&... shaders) noexcept
{
    if (!(shaders.isValid() && ...)) {
        NX_LOG(E, "GPU: Failed to create program; Invalid shaders");
        return false;
    }

    if constexpr (detail::BuildInfo::debug) {
        SDL_assert(validateShaderStages(shaders...));
    }

    mID = glCreateProgram();
    if (mID == 0) {
        NX_LOG(E, "GPU: Failed to create program object");
        return false;
    }

    (glAttachShader(mID, shaders.id()), ...);

    if (!linkProgram()) {
        NX_LOG(E, "GPU: Failed to link program");
        return false;
    }

    if (!createUniformCache()) {
        NX_LOG(E, "GPU: Failed to create uniform cache");
        return false;
    }

    return true;
}

inline bool Program::linkProgram() noexcept
{
    glLinkProgram(mID);

    GLint success;
    glGetProgramiv(mID, GL_LINK_STATUS, &success);

    if (!success) {
        GLint logLength;
        glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string errorLog(logLength, '\0');
            glGetProgramInfoLog(mID, logLength, nullptr, errorLog.data());
            NX_LOG(E, "GPU: Failed to link program: %s", errorLog.c_str());
        }
        else {
            NX_LOG(E, "GPU: Failed to link program (no error log available)");
        }
    }

    SDL_assert(success > 0);

    return static_cast<bool>(success);
}

template<typename... Shaders>
inline bool Program::validateShaderStages(const Shaders&... shaders) noexcept
{
    constexpr size_t count = sizeof...(Shaders);
    std::array<GLenum, count> stages = {shaders.stage()...};

    // Compute pipeline: only one compute shader
    if (std::find(stages.begin(), stages.end(), GL_COMPUTE_SHADER) != stages.end()) {
        if (count != 1) {
            NX_LOG(E, "GPU: Compute pipeline requires exactly one compute shader");
            return false;
        }
        return true;
    }

    // Graphics pipeline: checks
    bool hasVertex = std::find(stages.begin(), stages.end(), GL_VERTEX_SHADER) != stages.end();
    bool hasFragment = std::find(stages.begin(), stages.end(), GL_FRAGMENT_SHADER) != stages.end();
    if (!hasVertex || !hasFragment) {
        NX_LOG(E, "GPU: Graphics pipeline requires at least vertex and fragment shaders");
        return false;
    }

    // Check that there are no duplicates
    std::array<GLenum, count> sortedStages = stages;
    std::sort(sortedStages.begin(), sortedStages.end());
    if (std::adjacent_find(sortedStages.begin(), sortedStages.end()) != sortedStages.end()) {
        NX_LOG(E, "GPU: Duplicate shader stages detected");
        return false;
    }
    
    return true;
}

inline bool Program::createUniformCache() noexcept
{
    GLint numUniforms = 0;
    glGetProgramInterfaceiv(mID, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    GLenum props[] = {GL_TYPE, GL_LOCATION};
    GLint type = 0, location = 0;
    GLint maxLocation = -1;

    for (GLint i = 0; i < numUniforms; ++i)
    {
        GLint values[2];
        glGetProgramResourceiv(mID, GL_UNIFORM, i, 2, props, 2, nullptr, values);
        type = values[0];
        location = values[1];

        if(location > maxLocation) {
            maxLocation = location;
        }
    }

    if (maxLocation < 0) {
        return true;
    }

    size_t cacheSize = maxLocation + 1;
    mUniformCache = decltype(mUniformCache)(
        cacheSize, cacheSize
    );

    if (mUniformCache.capacity() != cacheSize) {
        return false;
    }

    // Init cache with sampler binding points
    for (GLint i = 0; i < numUniforms; ++i)
    {
        GLint values[2];
        glGetProgramResourceiv(mID, GL_UNIFORM, i, 2, props, 2, nullptr, values);
        type = values[0];
        location = values[1];

        bool isSampler = false;
        switch (type) {
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_BUFFER:
        case GL_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_IMAGE_CUBE:
        case GL_IMAGE_BUFFER:
        case GL_IMAGE_2D_ARRAY:
            isSampler = true;
            break;
        }

        if (isSampler && location >= 0) {
            GLint defaultBinding = 0;
            glGetUniformiv(mID, location, &defaultBinding);
            SDL_memcpy(&mUniformCache[location], &defaultBinding, sizeof(GLint));
        }
    }

    return true;
}

inline void Program::cleanup() noexcept
{
    if (mID != 0) {
        glDeleteProgram(mID);
        mID = 0;
    }
}

} // namespace gpu

#endif // NX_GPU_PROGRAM_HPP
