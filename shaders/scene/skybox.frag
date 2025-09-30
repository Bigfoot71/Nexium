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

/* === Varyings === */

layout(location = 0) in vec3 vPosition;

/* === Samplers === */

layout(binding = 0) uniform samplerCube uTexSkybox;

/* === Uniforms === */

layout(location = 1) uniform float uIntensity;
layout(location = 2) uniform float uFogAffect;
layout(location = 3) uniform vec3 uFogColor;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Programs === */

void main()
{
    vec3 color = texture(uTexSkybox, vPosition).rgb * uIntensity;
    FragColor = vec4(mix(color, uFogColor, uFogAffect), 1.0);
}
