#ifndef NX_RENDER_SHADER_3D_HPP
#define NX_RENDER_SHADER_3D_HPP

#include "./Core/ShaderOverride.hpp"
#include <NX/NX_Render.h>

/* === Shader Traits Specialization === */

/**
 * Defines the shader variants of NX_Shader3D.
 */
template <>
struct render::ShaderTraits<NX_Shader3D> {
    enum Variant {
        SCENE_LIT,        // Full PBR/Phong lighting
        SCENE_UNLIT,      // No lighting, just albedo
        SCENE_PREPASS,    // Depth/normal prepass
        SCENE_SHADOW,     // Shadow map generation
        VARIANT_COUNT
    };
};

/* === Declaration === */

class NX_Shader3D : public render::ShaderOverride<NX_Shader3D> {
public:
    /** Type aliases for convenience */
    using Variant = render::ShaderTraits<NX_Shader3D>::Variant;

public:
    /** Create default material shader with built-in shaders */
    NX_Shader3D();
    
    /** Create custom material shader with user-provided code */
    NX_Shader3D(const char* vertexCode, const char* fragmentCode);

    /** Get the appropriate shader program for a given shading mode */
    gpu::Program& programFromShadingMode(NX_ShadingMode shading);

private:
    /** Convert engine shading mode to internal shader variant */
    static Variant variantFromShadingMode(NX_ShadingMode shading);
};

/* === Inline Implementation === */

inline gpu::Program& NX_Shader3D::programFromShadingMode(NX_ShadingMode shading)
{
    return ShaderOverride::program(variantFromShadingMode(shading));
}

inline NX_Shader3D::Variant 
NX_Shader3D::variantFromShadingMode(NX_ShadingMode shading)
{
    switch (shading) {
    case NX_SHADING_LIT:
        return Variant::SCENE_LIT;
    case NX_SHADING_UNLIT:
        return Variant::SCENE_UNLIT;
    default:
        break;
    }

    return Variant::SCENE_LIT;
}

#endif // NX_RENDER_SHADER_3D_HPP
