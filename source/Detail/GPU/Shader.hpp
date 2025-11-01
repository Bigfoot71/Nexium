/* Shader.hpp -- Allows high-level GPU shader management
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_GPU_SHADER_HPP
#define NX_GPU_SHADER_HPP

#include "../../INX_GlobalState.hpp"     //< Used to get OpenGL profile used (Core/ES)
#include <NX/NX_Log.h>

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_video.h>
#include <glad/gles2.h>

#include <initializer_list>
#include <utility>
#include <string>

namespace gpu {

/* === Declaration === */

class Shader {
public:
    explicit Shader(GLenum stage, const char* source, std::initializer_list<const char*> defines = {}) noexcept;
    ~Shader() noexcept;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool isValid() const noexcept;
    GLuint id() const noexcept;
    GLenum stage() const noexcept;

private:
    GLuint mID{0};
    GLenum mStage;

private:
    static const char* stageToString(GLenum stage) noexcept;
};

/* === Public Implementation === */

inline Shader::Shader(GLenum stage, const char* source, std::initializer_list<const char*> defines) noexcept
    : mStage(stage)
{
    if (!source) {
        NX_LOG(E, "GPU: Failed to create shader; Source is null");
        SDL_assert(false);
        return;
    }

    /* --- Creating the shader --- */

    mID = glCreateShader(stage);
    if (mID == 0) {
        NX_LOG(E, "GPU: Failed to create shader object");
        SDL_assert(false);
        return;
    }

    /* --- Building final source with defines --- */

    std::string finalSource;

    if (INX_Display.glProfile == SDL_GL_CONTEXT_PROFILE_ES) {
        finalSource = "#version 320 es\n";
    }
    else {
        finalSource = "#version 450 core\n";
    }

    for (const char* define : defines) {
        if (define && strlen(define) > 0) {
            finalSource += "#define ";
            finalSource += define;
            finalSource += "\n";
        }
    }

    finalSource += source;

    /* --- Compiling the shader --- */

    const char* finalSourcePtr = finalSource.c_str();
    glShaderSource(mID, 1, &finalSourcePtr, nullptr);
    glCompileShader(mID);

    /* --- Compilation Check --- */

    GLint success;
    glGetShaderiv(mID, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLint logLength;
        glGetShaderiv(mID, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0) {
            std::string errorLog(logLength, '\0');
            glGetShaderInfoLog(mID, logLength, nullptr, errorLog.data());
            NX_LOG(E, "GPU: Failed to compile %s shader:\n%s", stageToString(stage), errorLog.c_str());
        }
        else {
            NX_LOG(E, "GPU: Failed to compile %s shader (no error log available)", stageToString(stage));
        }

        glDeleteShader(mID);
        mID = 0;

        SDL_assert(false);
    }
}

inline Shader::~Shader() noexcept
{
    if (mID != 0) {
        glDeleteShader(mID);
        mID = 0;
    }
}

inline Shader::Shader(Shader&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mStage(other.mStage)
{ }

inline Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other) {
        if (mID != 0) {
            glDeleteShader(mID);
        }
        mID = std::exchange(other.mID, 0);
        mStage = other.mStage;
    }
    return *this;
}

inline bool Shader::isValid() const noexcept
{
    return (mID > 0);
}

inline GLuint Shader::id() const noexcept
{
    return mID;
}

inline GLenum Shader::stage() const noexcept
{
    return mStage;
}

inline const char* Shader::stageToString(GLenum stage) noexcept
{
    switch (stage) {
    case GL_VERTEX_SHADER:          return "vertex";
    case GL_GEOMETRY_SHADER:        return "geometry";
    case GL_TESS_CONTROL_SHADER:    return "tesselation control";
    case GL_TESS_EVALUATION_SHADER: return "tesselation evaluation";
    case GL_FRAGMENT_SHADER:        return "fragment";
    case GL_COMPUTE_SHADER:         return "compute";
    default:                        return "unknown";
    }
}

} // namespace gpu

#endif // NX_GPU_SHADER_HPP
