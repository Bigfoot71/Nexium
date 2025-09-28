/* shadow.frag -- Fragment shader for rendering shadow maps
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
#endif

/* === Varyings === */

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in float vAlpha;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;

/* === Uniforms === */

layout(location = 10) uniform vec3 uLightPosition;
layout(location = 11) uniform float uAlphaCutOff;
layout(location = 12) uniform float uLambda;
layout(location = 13) uniform float uFar;

/* === Fragments === */

layout(location = 0) out vec4 FragDistance;

/* === Program === */

void main()
{
    float alpha = vAlpha * texture(uTexAlbedo, vTexCoord).a;
    if (alpha < uAlphaCutOff) discard;

    // Normalized linear distance in [0,1]
    float d01 = length(vPosition - uLightPosition) / uFar;

#ifdef GL_ES
    // VSM
    float m1 = d01;
    float m2 = d01 * d01;
    FragDistance = vec4(m1, m2, 0.0, 1.0);
#else
    // EVSM
    float pExp = exp(+uLambda * d01);
    float nExp = exp(-uLambda * d01);
    FragDistance = vec4(pExp, nExp, 0.0, 1.0);
#endif
}
