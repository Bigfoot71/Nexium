/*
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

/* === Includes === */

#include "../include/math.glsl"

/* === Constants === */

const vec3 cubePositions[8] = vec3[]
(
    vec3(-0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5,  0.5)
);

const int cubeIndices[36] = int[]
(
    4, 5, 6,  4, 6, 7,  // (Z+)
    1, 0, 3,  1, 3, 2,  // (Z-)
    0, 4, 7,  0, 7, 3,  // (X-)
    5, 1, 2,  5, 2, 6,  // (X+)
    0, 1, 5,  0, 5, 4,  // (Y-)
    3, 7, 6,  3, 6, 2   // (Y+)
);

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform ViewFrustum {
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 invViewProj;
    mat4 invView;
    mat4 invProj;
    vec3 position;
    uint cullMask;
    float near;
    float far;
} uFrustum;

/* === Uniforms === */

layout(location = 0) uniform vec4 uRotation;

/* === Varyings === */

layout(location = 0) out vec3 vPosition;

/* === Program === */

void main()
{
    int vertexIndex = cubeIndices[gl_VertexID];
    vec3 position = cubePositions[vertexIndex];

    vPosition = M_Rotate3D(position, uRotation);
    gl_Position = uFrustum.proj * mat4(mat3(uFrustum.view)) * vec4(vPosition, 1.0);
}
