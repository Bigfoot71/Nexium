/* scene_prepass.frag -- Fragment shader for rendering shadow maps
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

/* === Defines === */

#define SHADOW      //< Definition for frame.glsl

/* === Includes === */

#include "../include/frame.glsl"

/* === Varyings === */

layout(location = 0) in VaryInternal {
    vec3 position;
    vec2 texCoord;
    vec4 color;
    mat3 tbn;
} vInt;

layout(location = 10) in VaryUser {
    smooth vec4 data4f;
    flat ivec4 data4i;
} vUsr;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Frame {
    Frame uFrame;
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
    float alpha = vInt.color.a * texture(uTexAlbedo, vInt.texCoord).a;
    if (alpha < uMaterial.alphaCutOff) discard;

    // Normalized linear distance in [0,1]
    float d01 = length(vInt.position - uFrame.lightPosition) / uFrame.lightRange;

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
