/* skybox.frag -- Skybox fragment shader
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

layout(location = 0) in vec3 vPosition;

/* === Samplers === */

layout(binding = 0) uniform samplerCube uTexSkybox;

/* === Uniform Buffers === */

layout(std140, binding = 2) uniform U_Environment {
    Environment uEnv;
};

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Programs === */

void main()
{
    vec3 color = texture(uTexSkybox, vPosition).rgb * uEnv.skyIntensity;
    FragColor = vec4(mix(color, uEnv.fogColor, uEnv.fogSkyAffect), 1.0);
}
