/* shape.frag -- Shape fragment shader override template
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
#define POSITION        vPosition
#define TEXCOORD        vTexCoord

/* === Outputs === */

vec4 COLOR = vec4(1.0);

/* === Override === */

#define fragment() //< The macro can be replaced by user code

void FragmentOverride()
{
    COLOR = vColor;

#if defined(SHAPE_TEXTURE)
    COLOR *= texture(uTexture, TEXCOORD);
#elif defined(TEXT_BITMAP)
    COLOR.a *= texture(uTexture, TEXCOORD).r;
#elif defined(TEXT_SDF)
    float sdf = texture(uTexture, TEXCOORD).r;
    float smoothing = fwidth(sdf);
    COLOR.a *= smoothstep(0.5 - smoothing, 0.5 + smoothing, sdf);
#endif

    fragment();
}
