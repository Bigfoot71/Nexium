/* bloom_downsample.frag -- Custom 36-tap bilinear downsampling shader for bloom generation
 *
 * Original implementation by Jorge Jiménez, presented at SIGGRAPH 2014
 * (used in Call of Duty: Advanced Warfare)
 *
 * Copyright (c) 2014 Jorge Jiménez
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
#endif

/* === Includes === */

#include "../include/environment.glsl"
#include "../include/color.glsl"

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexture;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Environment {
    Environment uEnv;
};

/* === Uniforms === */

layout(location = 0) uniform vec2 uTexelSize;        //< Reciprocal of the resolution of the source being sampled
layout(location = 1) uniform int uMipLevel;          //< Which mip we are writing to, used for Karis average

/* === Fragments === */

layout (location = 0) out vec3 FragColor;

/* === Helper Functions === */

float KarisAverage(vec3 col)
{
    float luma = C_LumaFromSRGB(C_LinearToSRGB(col)) * 0.25;
    return 1.0 / (1.0 + luma);
}

vec3 Prefilter(vec3 col)
{
	float brightness = max(col.r, max(col.g, col.b));
	float soft = brightness - uEnv.bloomPrefilter.y;
	soft = clamp(soft, 0.0, uEnv.bloomPrefilter.z);
	soft = soft * soft * uEnv.bloomPrefilter.w;
	float contribution = max(soft, brightness - uEnv.bloomPrefilter.x);
	contribution /= max(brightness, 0.00001);
	return col * contribution;
}

/* === Program === */

void main()
{
    // NOTE: This is the readable version of this shader. It will be optimized!

    float x = uTexelSize.x;
    float y = uTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(uTexture, vec2(vTexCoord.x - 2.0 * x, vTexCoord.y + 2.0 * y)).rgb;
    vec3 b = texture(uTexture, vec2(vTexCoord.x,           vTexCoord.y + 2.0 * y)).rgb;
    vec3 c = texture(uTexture, vec2(vTexCoord.x + 2.0 * x, vTexCoord.y + 2.0 * y)).rgb;

    vec3 d = texture(uTexture, vec2(vTexCoord.x - 2.0 * x, vTexCoord.y)).rgb;
    vec3 e = texture(uTexture, vec2(vTexCoord.x,           vTexCoord.y)).rgb;
    vec3 f = texture(uTexture, vec2(vTexCoord.x + 2.0 * x, vTexCoord.y)).rgb;

    vec3 g = texture(uTexture, vec2(vTexCoord.x - 2.0 * x, vTexCoord.y - 2.0 * y)).rgb;
    vec3 h = texture(uTexture, vec2(vTexCoord.x,           vTexCoord.y - 2.0 * y)).rgb;
    vec3 i = texture(uTexture, vec2(vTexCoord.x + 2.0 * x, vTexCoord.y - 2.0 * y)).rgb;

    vec3 j = texture(uTexture, vec2(vTexCoord.x - x, vTexCoord.y + y)).rgb;
    vec3 k = texture(uTexture, vec2(vTexCoord.x + x, vTexCoord.y + y)).rgb;
    vec3 l = texture(uTexture, vec2(vTexCoord.x - x, vTexCoord.y - y)).rgb;
    vec3 m = texture(uTexture, vec2(vTexCoord.x + x, vTexCoord.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    // Check if we need to perform Karis average on each block of 4 samples
    vec3 groups[5];
    if (uMipLevel == 0)
    {
        // We are writing to mip 0, so we need to apply Karis average to each block
        // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
        // artifacts).
        groups[0] = (a+b+d+e) * (0.125/4.0);
        groups[1] = (b+c+e+f) * (0.125/4.0);
        groups[2] = (d+e+g+h) * (0.125/4.0);
        groups[3] = (e+f+h+i) * (0.125/4.0);
        groups[4] = (j+k+l+m) * (0.5/4.0);
        groups[0] *= KarisAverage(groups[0]);
        groups[1] *= KarisAverage(groups[1]);
        groups[2] *= KarisAverage(groups[2]);
        groups[3] *= KarisAverage(groups[3]);
        groups[4] *= KarisAverage(groups[4]);
        FragColor = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
        FragColor = max(FragColor, 0.0001);
        FragColor = Prefilter(FragColor);
    }
    else
    {
        FragColor = e*0.125;                // ok
        FragColor += (a+c+g+i)*0.03125;     // ok
        FragColor += (b+d+f+h)*0.0625;      // ok
        FragColor += (j+k+l+m)*0.125;       // ok
    }
}
