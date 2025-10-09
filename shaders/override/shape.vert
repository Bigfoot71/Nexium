/* shape.vert -- Shape vertex shader override template
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Includes === */

#include "../include/math.glsl"

/* === Constants === */

#define TIME            uTime
#define VARYINGF        vUsrData4f
#define VARYINGI        vUsrData4i

/* === Outputs === */

vec2 POSITION   = vec2(0.0);
vec2 TEXCOORD   = vec2(0.0);
vec4 COLOR      = vec4(1.0);

/* === Override === */

#define vertex() //< The macro can be replaced by user code

void VertexOverride()
{
    POSITION = aPosition;
    TEXCOORD = aTexCoord;
    COLOR = aColor;

    vertex();
}
