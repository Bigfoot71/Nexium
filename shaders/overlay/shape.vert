/* shape.vert -- Shape vertex shader for overlay rendering
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

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform UniformBlock {
    mat4 uProjection;
    float uTime;
};

/* === Varyings === */

layout(location = 0) out vec2 vPosition;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out vec4 vColor;

layout(location = 3) out smooth vec4 vUsrData4f;
layout(location = 4) out flat ivec4 vUsrData4i;

/* === Vertex Override === */

#include "../override/shape.vert"

/* === Program === */

void main()
{
    VertexOverride();

    vPosition = POSITION;
    vTexCoord = TEXCOORD;
    vColor = COLOR;

    gl_Position = uProjection * vec4(vec3(POSITION, 0.0), 1.0);
}
