/* scene.vert -- Scene vertex shader override template
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Inputs === */

#define INSTANCE_DATA   iCustom

/* === Outputs === */

vec3 POSITION   = vec3(0.0);
vec2 TEXCOORD   = vec2(0.0);
vec4 COLOR      = vec4(1.0);
vec3 NORMAL     = vec3(0.0, 0.0, 1.0);
vec4 TANGENT    = vec4(0.0, 0.0, 0.0, 1.0);

/* === Override === */

#define vertex() //< The macro can be replaced by user code

void VertexOverride()
{
    POSITION = aPosition;
    TEXCOORD = uMaterial.texOffset + aTexCoord * uMaterial.texScale;
    COLOR = aColor * iColor;
    NORMAL = aNormal;
    TANGENT = aTangent;

    vertex();
}
