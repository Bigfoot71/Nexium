/* cubemap_prefilter.frag -- Prefiltered cubemap generation fragment shader
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

#include "../include/math.glsl"

/* === Constants === */

#define EPSILON 1e-7

/* === Attributes === */

layout(location = 0) in vec3 vPosition;

/* === Samplers === */

layout(binding = 0) uniform samplerCube uTexCubemap;

/* === Uniforms === */

layout(location = 1) uniform float uCubeResolution;     //< Resolution of source cubemap (per face)
layout(location = 2) uniform int uMaxCubeMipLevel;      //< Maximum mipmap level in source cubemap
layout(location = 3) uniform float uRoughness;          //< Roughness level for each mip level (dst)

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Helper functions === */

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return nom / max(denom, EPSILON);
}

float RadicalInverse_VdC(uint bits)
{
    // Efficient VanDerCorpus calculation
    // See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = M_TAU * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(max(1.0 - cosTheta * cosTheta, 0.0));

    // Spherical coordinates -> Cartesian (halfway vector)
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // Transformation to world space
    mat3 TBN = M_OrthonormalBasis(N);
    return normalize(TBN * H);
}

float ComputeMipLevel(float roughness, float pdf, float cubeResolution, uint sampleCount)
{
    float saTexel = 4.0 * M_PI / (6.0 * cubeResolution * cubeResolution);
    float saSample = 1.0 / (float(sampleCount) * max(pdf, EPSILON));
    return max(0.0, 0.5 * log2(saSample / saTexel));
}

/* === Program === */

void main()
{
    vec3 N = normalize(vPosition);
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    // Handle the case where roughness = 0 (perfect mirror)
    if (uRoughness < EPSILON) {
        FragColor = vec4(textureLod(uTexCubemap, N, 0.0).rgb, 1.0);
        return;
    }

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, uRoughness);
        vec3 L = normalize(reflect(-V, H));

        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > EPSILON)
        {
            float D = DistributionGGX(N, H, uRoughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);

            float pdf = (D * NdotH) / max(4.0 * HdotV, EPSILON);
            float mipLevel = ComputeMipLevel(uRoughness, pdf, uCubeResolution, SAMPLE_COUNT);
            mipLevel = clamp(mipLevel, 0.0, float(uMaxCubeMipLevel - 1));

            vec3 sampleColor = textureLod(uTexCubemap, L, mipLevel).rgb;
            prefilteredColor += sampleColor * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / max(totalWeight, EPSILON);
    FragColor = vec4(prefilteredColor, 1.0);
}
