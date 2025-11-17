/* edge_aware_blur.frag -- Bilateral blur (depth + normal aware)
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

// Universal bilateral blur shader that works for both SSAO and SSGI.
// Uses both depth and normal information to preserve edges while denoising.

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
#endif

/* === Includes === */

#include "../include/environment.glsl"
#include "../include/frustum.glsl"
#include "../include/math.glsl"

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexColor;
layout(binding = 1) uniform sampler2D uTexNormal;
layout(binding = 2) uniform sampler2D uTexDepth;

/* === Uniform Buffers === */

layout(std140, binding = 1) uniform U_ViewFrustum {
    Frustum uFrustum;
};

/* === Uniforms === */

layout(location = 0) uniform vec2 uBlurDirection;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Blur Coefficients === */

// NOTE: Generated using https://lisyarus.github.io/blog/posts/blur-coefficients-generator.html

// Parameters:
//  - Radius: 5.0
//  - Sigma: 3.0

const int SAMPLE_COUNT = 6;

const float OFFSETS[6] = float[6](
    -4.378621204796657,
    -2.431625915613778,
    -0.4862426846689485,
    1.4588111840004858,
    3.4048471718931532,
    5
);

const float WEIGHTS[6] = float[6](
    0.09461172151436463,
    0.20023097066826712,
    0.2760751120037518,
    0.24804559825032563,
    0.14521459357563646,
    0.035822003987654526
);


const float NORMAL_THRESHOLD = 0.85;    // Cosine similarity threshold
const float DEPTH_SENSITIVITY = 0.5;    // Controls depth edge preservation

/* === Helper Functions === */

vec3 ViewPositionFromDepth(vec2 uv, float depth)
{
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = uFrustum.invProj * clipPos;
    return viewPos.xyz / viewPos.w;
}

/* === Main Program === */

void main()
{
    vec4 centerColor = texture(uTexColor, vTexCoord);
    float centerDepth = texture(uTexDepth, vTexCoord).r;

    if (centerDepth > 0.9999) {
        FragColor = centerColor;
        return;
    }

    vec3 centerNormal = M_DecodeOctahedral(texture(uTexNormal, vTexCoord).rg);
    vec3 centerViewPos = ViewPositionFromDepth(vTexCoord, centerDepth);

    vec2 texelSize = 1.0 / vec2(textureSize(uTexColor, 0));
    vec2 texelDir = uBlurDirection * texelSize;

    vec4 result = vec4(0.0);
    float totalWeight = 0.0;

    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        vec2 sampleUV = vTexCoord + texelDir * OFFSETS[i];
        if (any(lessThan(sampleUV, vec2(0.0))) || any(greaterThan(sampleUV, vec2(1.0)))) {
            continue;
        }

        float sampleDepth = texture(uTexDepth, sampleUV).r;
        if (sampleDepth > 0.9999) {
            continue;
        }

        vec4 sampleColor = texture(uTexColor, sampleUV);
        vec3 sampleNormal = M_DecodeOctahedral(texture(uTexNormal, sampleUV).rg);
        vec3 sampleViewPos = ViewPositionFromDepth(sampleUV, sampleDepth);

        float normalSimilarity = max(0.0, dot(centerNormal, sampleNormal));
        float normalWeight = step(NORMAL_THRESHOLD, normalSimilarity);

        float depthDiff = abs(centerViewPos.z - sampleViewPos.z);
        float depthWeight = exp(-depthDiff / DEPTH_SENSITIVITY);

        float weight = WEIGHTS[i] * normalWeight * depthWeight;
        result += sampleColor * weight;
        totalWeight += weight;
    }

    FragColor = totalWeight > 0.001 ? result / totalWeight : centerColor;
}
