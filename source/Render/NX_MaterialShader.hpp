#ifndef NX_RENDER_MATERIAL_SHADER_HPP
#define NX_RENDER_MATERIAL_SHADER_HPP

#include "./Core/ShaderOverride.hpp"
#include <NX/NX_Render.h>

/* === Shader Traits Specialization === */

/**
 * Defines the shader variants of NX_MaterialShader.
 */
template <>
struct render::ShaderTraits<NX_MaterialShader> {
    enum Variant {
        SCENE_LIT,        // Full PBR/Phong lighting
        SCENE_UNLIT,      // No lighting, just albedo
        SCENE_WIREFRAME,  // Wireframe rendering
        SCENE_PREPASS,    // Depth/normal prepass
        SCENE_SHADOW,     // Shadow map generation
        VARIANT_COUNT
    };
};

/* === Declaration === */

class NX_MaterialShader : public render::ShaderOverride<NX_MaterialShader> {
public:
    /** Type aliases for convenience */
    using Variant = render::ShaderTraits<NX_MaterialShader>::Variant;

public:
    /** Create default material shader with built-in shaders */
    NX_MaterialShader();
    
    /** Create custom material shader with user-provided code */
    NX_MaterialShader(const char* vertexCode, const char* fragmentCode);

    /** Get the appropriate shader program for a given shading mode */
    gpu::Program& programFromShadingMode(NX_ShadingMode shading);

private:
    /** Convert engine shading mode to internal shader variant */
    static Variant variantFromShadingMode(NX_ShadingMode shading);
};

/* === Inline Implementation === */

inline gpu::Program& NX_MaterialShader::programFromShadingMode(NX_ShadingMode shading)
{
    return ShaderOverride::program(variantFromShadingMode(shading));
}

inline NX_MaterialShader::Variant 
NX_MaterialShader::variantFromShadingMode(NX_ShadingMode shading)
{
    switch (shading) {
    case NX_SHADING_LIT:
        return Variant::SCENE_LIT;
    case NX_SHADING_UNLIT:
        return Variant::SCENE_UNLIT;
    case NX_SHADING_WIREFRAME:
        return Variant::SCENE_WIREFRAME;
    default:
        return Variant::SCENE_LIT;
    }
}

#endif // NX_RENDER_MATERIAL_SHADER_HPP
