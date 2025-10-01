/* bloom_post.frag -- Fragment shader for applying bloom to the scene
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

#include "../include/environment.glsl"

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexColor;
layout(binding = 1) uniform sampler2D uTexBloom;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Environment {
    Environment uEnv;
};

/* === Fragments === */

out vec3 FragColor;

/* === Main program === */

void main()
{
    vec3 color = texture(uTexColor, vTexCoord).rgb;
    vec3 bloom = texture(uTexBloom, vTexCoord).rgb;
    bloom *= uEnv.bloomStrength;

#if defined(BLOOM_MIX)
    color = mix(color, bloom, uEnv.bloomStrength);
#elif defined(BLOOM_ADDITIVE)
    color += bloom;
#elif defined(BLOOM_SCREEN)
    bloom = clamp(bloom, vec3(0.0), vec3(1.0));
    color = max((color + bloom) - (color * bloom), vec3(0.0));
#endif

    FragColor = vec3(color);
}
