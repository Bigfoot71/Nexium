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

/* === Includes === */

#include "../include/frame.glsl"

/* === Varyings === */

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in float vAlpha;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Frame {
    FrameShadow uFrame;
};

layout(std140, binding = 4) uniform U_Material {
    vec4 albedoColor;
    vec3 emissionColor;
    float emissionEnergy;
    float aoLightAffect;
    float occlusion;
    float roughness;
    float metalness;
    float normalScale;
    float alphaCutOff;
    vec2 texOffset;
    vec2 texScale;
    int billboard;
} uMaterial;

/* === Fragments === */

layout(location = 0) out vec4 FragDistance;

/* === Program === */

void main()
{
    float alpha = vAlpha * texture(uTexAlbedo, vTexCoord).a;
    if (alpha < uMaterial.alphaCutOff) discard;

    // Normalized linear distance in [0,1]
    float d01 = length(vPosition - uFrame.lightPosition) / uFrame.farPlane;

#ifdef GL_ES
    // VSM
    float m1 = d01;
    float m2 = d01 * d01;
    FragDistance = vec4(m1, m2, 0.0, 1.0);
#else
    // EVSM
    float pExp = exp(+uFrame.shadowLambda * d01);
    float nExp = exp(-uFrame.shadowLambda * d01);
    FragDistance = vec4(pExp, nExp, 0.0, 1.0);
#endif
}
