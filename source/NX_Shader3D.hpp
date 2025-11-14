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
#include <NX/NX_Material.h>

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

/**
 * Defines the shader variants of NX_Shader3D.
 */
template <>
struct INX_ShaderTraits<NX_Shader3D> {
    enum Variant {
        LIT_GENERIC,    // Generic forward, opaque or transparent
        LIT_PREPASS,    // For opaque lit after pre-pass
        UNLIT,          // No lighting, just albedo
        PREPASS,        // Depth/normal prepass
        SHADOW,         // Shadow map generation
        VARIANT_COUNT
    };
};

class NX_Shader3D : public INX_Shader<NX_Shader3D> {
public:
    using Variant = INX_ShaderTraits<NX_Shader3D>::Variant;

public:
    NX_Shader3D(); //< Creates default shader
    NX_Shader3D(const char* vertexCode, const char* fragmentCode);
    const gpu::Program& GetProgramFromMaterial(const NX_Material& material, bool prepass = false) const;
};

inline const gpu::Program& NX_Shader3D::GetProgramFromMaterial(const NX_Material& material, bool prepass) const
{
    Variant variant = Variant::LIT_GENERIC;

    if (material.shading == NX_SHADING_UNLIT) {
        variant = Variant::UNLIT;
    }
    else if (prepass) {
        variant = Variant::LIT_PREPASS;
    }

    return INX_Shader::GetProgram(variant);
}

#endif // NX_RENDER_SHADER_3D_HPP
