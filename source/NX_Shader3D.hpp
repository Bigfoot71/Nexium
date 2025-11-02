/* NX_Shader3D.hpp -- API definition for Nexium's 3D shader module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_RENDER_SHADER_3D_HPP
#define NX_RENDER_SHADER_3D_HPP

#include "./INX_Shader.hpp"
#include <NX/NX_Render.h>

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

/**
 * Defines the shader variants of NX_Shader3D.
 */
template <>
struct INX_ShaderTraits<NX_Shader3D> {
    enum Variant {
        SCENE_LIT,        // Full PBR lighting
        SCENE_UNLIT,      // No lighting, just albedo
        SCENE_PREPASS,    // Depth/normal prepass
        SCENE_SHADOW,     // Shadow map generation
        VARIANT_COUNT
    };
};

class NX_Shader3D : public INX_Shader<NX_Shader3D> {
public:
    using Variant = INX_ShaderTraits<NX_Shader3D>::Variant;

public:
    NX_Shader3D(); //< Creates default shader
    NX_Shader3D(const char* vertexCode, const char* fragmentCode);
    const gpu::Program& GetProgramFromShadingMode(NX_ShadingMode shading) const;
};

inline const gpu::Program& NX_Shader3D::GetProgramFromShadingMode(NX_ShadingMode shading) const
{
    Variant variant = Variant::SCENE_LIT;

    switch (shading) {
    case NX_SHADING_LIT: variant = Variant::SCENE_LIT; break;
    case NX_SHADING_UNLIT: variant = Variant::SCENE_UNLIT; break;
    default: break;
    }

    return INX_Shader::GetProgram(variant);
}

#endif // NX_RENDER_SHADER_3D_HPP
