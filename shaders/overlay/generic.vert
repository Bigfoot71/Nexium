/* generic.vert -- Generic vertex shader for overlay rendering
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

/* === Attributes === */

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

/* === Uniforms === */

layout(std140, binding = 0) uniform UniformBlock {
    mat4 uProjection;
};

/* === Varyings === */

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

/* === Program === */

void main()
{
    vTexCoord = aTexCoord;
    vColor = aColor;

    gl_Position = uProjection * vec4(vec3(aPosition, 0.0), 1.0);
}
