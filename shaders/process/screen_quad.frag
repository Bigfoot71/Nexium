/* screen.vert -- Generic fragment shader for rendering a full-screen quad
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

/* === Varying === */

layout(location = 0) in vec2 vTexCoord;

/* === Uniforms === */

layout(location = 0) uniform vec4 uValue;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Program === */

void main()
{
    FragColor = uValue;
}
