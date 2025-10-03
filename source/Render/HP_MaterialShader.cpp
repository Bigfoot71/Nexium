#include "./HP_MaterialShader.hpp"

#include <shaders/forward.vert.h>
#include <shaders/forward.frag.h>
#include <shaders/prepass.vert.h>
#include <shaders/prepass.frag.h>
#include <shaders/shadow.vert.h>
#include <shaders/shadow.frag.h>

#include <string_view>

/* === Public Implementation === */

HP_MaterialShader::HP_MaterialShader()
    : mForward(
        gpu::Shader(GL_VERTEX_SHADER, FORWARD_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, FORWARD_FRAG)
    )
    , mPrePass(
        gpu::Shader(GL_VERTEX_SHADER, PREPASS_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, PREPASS_FRAG)
    )
    , mShadow(
        gpu::Shader(GL_VERTEX_SHADER, SHADOW_VERT),
        gpu::Shader(GL_FRAGMENT_SHADER, SHADOW_FRAG)
    )
{ }

HP_MaterialShader::HP_MaterialShader(const char* vert, const char* frag)
{
    /* --- Constants --- */

    constexpr std::string_view vertDefine = "#define vertex()";
    constexpr std::string_view fragDefine = "#define fragment()";

    /* --- Load all base shaders --- */

    std::string vertForward = FORWARD_VERT;
    std::string fragForward = FORWARD_FRAG;
    std::string vertPrepass = PREPASS_VERT;
    std::string fragPrepass = PREPASS_FRAG;
    std::string vertShadow = SHADOW_VERT;
    std::string fragShadow = SHADOW_FRAG;

    /* --- Process shaders --- */

    processCode(vertForward, vertDefine, vert);
    processCode(fragForward, fragDefine, frag);
    processCode(vertForward, vertDefine, vert);
    processCode(fragForward, fragDefine, frag);
    processCode(vertForward, vertDefine, vert);
    processCode(fragForward, fragDefine, frag);

    /* --- Compile shaders --- */

    mForward = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertForward.c_str()),
        gpu::Shader(GL_FRAGMENT_SHADER, fragForward.c_str())
    );

    mPrePass = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertPrepass.c_str()),
        gpu::Shader(GL_FRAGMENT_SHADER, fragPrepass.c_str())
    );

    mShadow = gpu::Program(
        gpu::Shader(GL_VERTEX_SHADER, vertShadow.c_str()),
        gpu::Shader(GL_FRAGMENT_SHADER, fragShadow.c_str())
    );
}

/* === Private Implementation === */

void HP_MaterialShader::processCode(std::string& source, const std::string_view& define, const char* code)
{
    if (code == nullptr) {
        return;
    }

    size_t pos = source.find(define);

    if (pos != std::string::npos) {
        source.replace(pos, define.length(), code);
    }
}
