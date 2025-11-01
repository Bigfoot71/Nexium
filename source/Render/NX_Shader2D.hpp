#ifndef NX_RENDER_SHADER_2D_HPP
#define NX_RENDER_SHADER_2D_HPP

#include "./Core/ShaderOverride.hpp"
#include <NX/NX_Render.h>

/* === Shader Traits Specialization === */

/**
 * Defines the shader variants of NX_Shader2D.
 */
template <>
struct render::ShaderTraits<NX_Shader2D> {
    enum Variant {
        SHAPE_COLOR,
        SHAPE_TEXTURE,
        TEXT_BITMAP,
        TEXT_SDF,
        VARIANT_COUNT
    };
};

/* === Declaration === */

class NX_Shader2D : public render::ShaderOverride<NX_Shader2D> {
public:
    /** Type aliases for convenience */
    using Variant = render::ShaderTraits<NX_Shader2D>::Variant;

public:
    /** Create default shader with built-in shaders */
    NX_Shader2D();
    
    /** Create custom shader with user-provided code */
    NX_Shader2D(const char* vertexCode, const char* fragmentCode);
};

#endif // NX_RENDER_SHADER_HPP
