/* screen.vert -- Generic vertex shader for rendering a full-screen quad
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

/* === Constants === */

const vec2 positions[3] = vec2[]
(
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

/* === Varying === */

layout(location = 0) out vec2 vTexCoord;

/* === Program === */

void main()
{
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    vTexCoord = (gl_Position.xy * 0.5) + 0.5;
}
