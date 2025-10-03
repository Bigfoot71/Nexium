/* prepass.frag -- Fragment shader for depth pre-pass
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

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in float vAlpha;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;

/* === Uniforms === */

layout(std140, binding = 3) uniform U_Material {
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

/* === Program === */

void main()
{
    float alpha = vAlpha * texture(uTexAlbedo, vTexCoord).a;
    if (alpha < uMaterial.alphaCutOff) discard;
}
