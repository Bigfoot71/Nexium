#ifndef HP_RENDER_MATERIAL_SHADER_HPP
#define HP_RENDER_MATERIAL_SHADER_HPP

#include "../Detail/GPU/Program.hpp"

/* === Declaration === */

class HP_MaterialShader {
public:
    HP_MaterialShader();
    HP_MaterialShader(const char* vertex, const char* fragment);

    /** Getters */
    gpu::Program& forward();
    gpu::Program& prepass();
    gpu::Program& shadow();

private:
    void processCode(std::string& source, const std::string_view& define, const char* code);

private:
    gpu::Program mForward;
    gpu::Program mPrePass;
    gpu::Program mShadow;
};

/* === Public Implementation === */

inline gpu::Program& HP_MaterialShader::forward()
{
    return mForward;
}

inline gpu::Program& HP_MaterialShader::prepass()
{
    return mPrePass;
}

inline gpu::Program& HP_MaterialShader::shadow()
{
    return mShadow;
}

#endif // HP_RENDER_MATERIAL_SHADER_HPP
