/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
#endif

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexColor;
layout(binding = 1) uniform sampler2D uTexBloom;

/* === Uniforms === */

layout(location = 0) uniform float uStrength;

/* === Fragments === */

out vec3 FragColor;

/* === Main program === */

void main()
{
    vec3 color = texture(uTexColor, vTexCoord).rgb;
    vec3 bloom = texture(uTexBloom, vTexCoord).rgb;
    bloom *= uStrength;

#if defined(BLOOM_MIX)
    color = mix(color, bloom, uStrength);
#elif defined(BLOOM_ADDITIVE)
    color += bloom;
#elif defined(BLOOM_SCREEN)
    bloom = clamp(bloom, vec3(0.0), vec3(1.0));
    color = max((color + bloom) - (color * bloom), vec3(0.0));
#endif

    FragColor = vec3(color);
}
