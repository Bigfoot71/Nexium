#ifndef NX_RENDER_SHADER_HPP
#define NX_RENDER_SHADER_HPP

#include "./Core/ShaderOverride.hpp"
#include <NX/NX_Render.h>

/* === Shader Traits Specialization === */

/**
 * Defines the shader variants of NX_Shader.
 */
template <>
struct render::ShaderTraits<NX_Shader> {
    enum Variant {
        SHAPE_COLOR,
        SHAPE_TEXTURE,
        TEXT_BITMAP,
        TEXT_SDF,
        VARIANT_COUNT
    };
};

/* === Declaration === */

class NX_Shader : public render::ShaderOverride<NX_Shader> {
public:
    /** Type aliases for convenience */
    using Variant = render::ShaderTraits<NX_Shader>::Variant;

public:
    /** Create default shader with built-in shaders */
    NX_Shader();
    
    /** Create custom shader with user-provided code */
    NX_Shader(const char* vertexCode, const char* fragmentCode);
};

#endif // NX_RENDER_SHADER_HPP
