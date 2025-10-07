/* shape.frag -- Shape fragment shader for overlay rendering
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision mediump float;
#endif

/* === Varyings === */

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;

layout(location = 3) in smooth vec4 vUsrData4f;
layout(location = 4) in flat ivec4 vUsrData4i;

/* === Samplers === */

#if !defined(SHAPE_COLOR)
layout(binding = 0) uniform sampler2D uTexture;
#endif

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform UniformBlock {
    mat4 uProjection;
    float uTime;
};

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Fragments Override === */

#include "../include/template/shape.frag"

/* === Program === */

void main()
{
    FragmentOverride();
    FragColor = vec4(COLOR.rgb * COLOR.a, COLOR.a);
}
