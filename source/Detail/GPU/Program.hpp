/* Program.hpp -- Allows high-level GPU program management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_GPU_PROGRAM_HPP
#define HP_GPU_PROGRAM_HPP

#include <Hyperion/HP_Math.h>
#include <SDL3/SDL_assert.h>

#include "../../Core/HP_InternalLog.hpp"
#include "../Util/FixedArray.hpp"
#include "./Shader.hpp"

#include <glad/gles2.h>
#include <cstring>
#include <string>
#include <array>

namespace gpu {

/* === Declaration === */

class Program {
public:
    static constexpr int MaxUniforms = 32;

public:
    Program() = default;
    explicit Program(const Shader& vertexShader, const Shader& fragmentShader) noexcept;
    explicit Program(const Shader& computeShader) noexcept;

    ~Program() noexcept;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    Program(Program&& other) noexcept;
    Program& operator=(Program&& other) noexcept;

    bool isValid() const noexcept;
    GLuint id() const noexcept;

private:
    friend class Pipeline;

private:
    void setUint1(int location, uint32_t value) const noexcept;
    void setUint2(int location, const HP_IVec2& value) const noexcept;
    void setUint3(int location, const HP_IVec3& value) const noexcept;
    void setUint4(int location, const HP_IVec4& value) const noexcept;
    void setInt1(int location, int value) const noexcept;
    void setInt2(int location, const HP_IVec2& value) const noexcept;
    void setInt3(int location, const HP_IVec3& value) const noexcept;
    void setInt4(int location, const HP_IVec4& value) const noexcept;
    void setFloat1(int location, float value) const noexcept;
    void setFloat2(int location, const HP_Vec2& value) const noexcept;
    void setFloat3(int location, const HP_Vec3& value) const noexcept;
    void setFloat3(int location, const HP_Color& value) const noexcept;
    void setFloat4(int location, const HP_Vec4& value) const noexcept;
    void setFloat4(int location, const HP_Quat& value) const noexcept;
    void setFloat4(int location, const HP_Color& value) const noexcept;
    void setMat3(int location, const HP_Mat3& value) const noexcept;
    void setMat3(int location, const HP_Mat4& value) const noexcept;
    void setMat4(int location, const HP_Mat4& value) const noexcept;

private:
    bool linkProgram() noexcept;
    bool createUniformCache() noexcept;

private:
    GLuint mID{0};

private:
    mutable util::FixedArray<std::array<uint8_t, 64>> mUniformCache{};
};

/* === Public Implementation === */

inline Program::Program(const Shader& vertexShader, const Shader& fragmentShader) noexcept
{
    if (!vertexShader.isValid() || !fragmentShader.isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create program; Invalid shaders");
        return;
    }

    if (vertexShader.stage() != GL_VERTEX_SHADER || fragmentShader.stage() != GL_FRAGMENT_SHADER) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create program; Incorrect shader stages for graphics pipeline");
        return;
    }

    mID = glCreateProgram();
    if (mID == 0) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create program object");
        return;
    }

    glAttachShader(mID, vertexShader.id());
    glAttachShader(mID, fragmentShader.id());

    if (!linkProgram()) {
        glDeleteProgram(mID);
        mID = 0;
    }

    if (!createUniformCache()) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create uniform cache");
        glDeleteProgram(mID);
        mID = 0;
    }
}

inline Program::Program(const Shader& computeShader) noexcept
{
    if (!computeShader.isValid()) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create program; invalid compute shader");
        return;
    }

    if (computeShader.stage() != GL_COMPUTE_SHADER) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create program; shader is not a compute shader");
        return;
    }

    mID = glCreateProgram();
    if (mID == 0) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create program object");
        return;
    }

    glAttachShader(mID, computeShader.id());

    if (!linkProgram()) {
        glDeleteProgram(mID);
        mID = 0;
    }

    if (!createUniformCache()) {
        HP_INTERNAL_LOG(E, "GPU: Failed to create uniform cache");
        glDeleteProgram(mID);
        mID = 0;
    }
}

inline Program::~Program() noexcept
{
    if (mID != 0) {
        glDeleteProgram(mID);
        mID = 0;
    }
}

inline Program::Program(Program&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mUniformCache(std::move(other.mUniformCache))
{ }

inline Program& Program::operator=(Program&& other) noexcept
{
    if (this != &other) {
        if (mID != 0) {
            glDeleteProgram(mID);
        }
        mID = std::exchange(other.mID, 0);
        mUniformCache = std::move(other.mUniformCache);
    }
    return *this;
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

inline void Program::setUint2(int location, const HP_IVec2& value) const noexcept
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

inline void Program::setUint3(int location, const HP_IVec3& value) const noexcept
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

inline void Program::setUint4(int location, const HP_IVec4& value) const noexcept
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

inline void Program::setInt2(int location, const HP_IVec2& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform2iv(location, 1, reinterpret_cast<const int*>(&value));
    }
}

inline void Program::setInt3(int location, const HP_IVec3& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform3iv(location, 1, reinterpret_cast<const int*>(&value));
    }
}

inline void Program::setInt4(int location, const HP_IVec4& value) const noexcept
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

inline void Program::setFloat2(int location, const HP_Vec2& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform2fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat3(int location, const HP_Vec3& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform3fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat3(int location, const HP_Color& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(HP_Vec3));
        glUniform3fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat4(int location, const HP_Vec4& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniform4fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setFloat4(int location, const HP_Quat& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    HP_Vec4 quat = { value.x, value.y, value.z, value.w };
    if (std::memcmp(mUniformCache[location].data(), &quat, sizeof(quat)) != 0) {
        std::memcpy(mUniformCache[location].data(), &quat, sizeof(quat));
        glUniform4fv(location, 1, reinterpret_cast<const float*>(&quat));
    }
}

inline void Program::setFloat4(int location, const HP_Color& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(HP_Vec4));
        glUniform4fv(location, 1, reinterpret_cast<const float*>(&value));
    }
}

inline void Program::setMat3(int location, const HP_Mat3& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), value.a, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), value.a, sizeof(value));
        glUniformMatrix3fv(location, 1, GL_FALSE, value.a);
    }
}

inline void Program::setMat3(int location, const HP_Mat4& value) const noexcept
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

inline void Program::setMat4(int location, const HP_Mat4& value) const noexcept
{
    SDL_assert(mUniformCache.capacity() > 0);
    SDL_assert(location < mUniformCache.size());

    if (std::memcmp(mUniformCache[location].data(), &value, sizeof(value)) != 0) {
        std::memcpy(mUniformCache[location].data(), &value, sizeof(value));
        glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const float*>(&value));
    }
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
            HP_INTERNAL_LOG(E, "GPU: Failed to link program: %s", errorLog.c_str());
        }
        else {
            HP_INTERNAL_LOG(E, "GPU: Failed to link program (no error log available)");
        }
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
            continue;
        }

        if(location > maxLocation) {
            maxLocation = location;
        }
    }

    if(maxLocation < 0) {
        return true;
    }

    size_t cacheSize = maxLocation + 1;
    mUniformCache = decltype(mUniformCache)(
        cacheSize, cacheSize
    );

    return (mUniformCache.capacity() == cacheSize);
}

} // namespace gpu

#endif // HP_GPU_PROGRAM_HPP
