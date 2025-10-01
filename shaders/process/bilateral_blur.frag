/* bilateral_blur.frag -- Bilateral depth-aware blur fragment shader for SSAO
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

// NOTE: The coefficients for the two-pass Gaussian blur were generated using:
//       https://lisyarus.github.io/blog/posts/blur-coefficients-generator.html

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
#endif

/* === Includes === */

#include "../include/utils.glsl"

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexColor;
layout(binding = 1) uniform sampler2D uTexDepth;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform ViewFrustum {
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 invViewProj;
    mat4 invView;
    mat4 invProj;
    vec3 position;
    uint cullMask;
    float near;
    float far;
} uFrustum;

/* === Uniforms === */

layout(location = 0) uniform vec2 uTexelDir;
layout(location = 1) uniform float uRadius;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Blur Coefs === */

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

/* === Main Program === */

void main()
{
    // This is a depth aware bilateral blur for SSAO that smooths noise while keeping edges intact.
    // A standard Gaussian blur would cause occlusion to "bleed" across depth discontinuities,
    // creating bright halos around objects. By reducing blur strength at depth edges, we
    // maintain sharp occlusion boundaries while still removing noise on flat surfaces.

    float centerDepth = U_LinearizeDepth(texture(uTexDepth, vTexCoord).r, uFrustum.near, uFrustum.far);

    vec4 result = vec4(0.0);
    float totalWeight = 0.0;

    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        vec2 sampleUV = vTexCoord + uTexelDir * OFFSETS[i];
        float sampleDepth = U_LinearizeDepth(texture(uTexDepth, sampleUV).r, uFrustum.near, uFrustum.far);
        float diff = abs(centerDepth - sampleDepth);

        // Modulate the Gaussian weight based on depth similarity:
        // - When diff = 0 (same depth): depthWeight = 1.0 (full blur)
        // - When diff >= uRadius (depth discontinuity): depthWeight = 0.1 (minimal blur)
        // The smoothstep provides a smooth transition, and the 0.1 minimum ensures we
        // always blur at least slightly to remove SSAO noise, even at edges.

        float depthWeight = mix(0.1, 1.0, smoothstep(uRadius, 0.0, diff));
        float weight = WEIGHTS[i] * depthWeight;

        result += texture(uTexColor, sampleUV) * weight;
        totalWeight += weight;
    }

    FragColor = result / totalWeight;
}
