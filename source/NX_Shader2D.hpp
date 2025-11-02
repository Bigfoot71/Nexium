#ifndef NX_SHADER_2D_HPP
#define NX_SHADER_2D_HPP

#include <NX/NX_Shader2D.h>
#include "./INX_Shader.hpp"

// ============================================================================
// OPAQUE DEFINITION
// ============================================================================

/**
 * Defines the shader variants of NX_Shader2D
 */
template <>
struct INX_ShaderTraits<NX_Shader2D> {
    enum Variant {
        SHAPE_COLOR,
        SHAPE_TEXTURE,
        TEXT_BITMAP,
        TEXT_SDF,
        VARIANT_COUNT
    };
};

class NX_Shader2D : public INX_Shader<NX_Shader2D> {
public:
    using Variant = INX_ShaderTraits<NX_Shader2D>::Variant;

public:
    NX_Shader2D();
    NX_Shader2D(const char* vertexCode, const char* fragmentCode);
};

#endif // NX_SHADER_2D_HPP
