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

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Programs === */

void main()
{
    FragColor = vec4(texture(uTexSkybox, vPosition).rgb, 1.0);
    FragColor.rgb *= uIntensity;
}
