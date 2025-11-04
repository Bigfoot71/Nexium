/* skybox.vert -- Skybox vertex shader
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
#include "../include/frustum.glsl"
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

layout(std140, binding = 1) uniform U_ViewFrustum {
    INX_Frustum uFrustum;
};

layout(std140, binding = 2) uniform U_Environment {
    Environment uEnv;
};

/* === Varyings === */

layout(location = 0) out vec3 vPosition;

/* === Program === */

void main()
{
    int vertexIndex = cubeIndices[gl_VertexID];
    vec3 position = cubePositions[vertexIndex];

    vPosition = M_Rotate3D(position, uEnv.skyRotation);
    gl_Position = uFrustum.proj * mat4(mat3(uFrustum.view)) * vec4(vPosition, 1.0);
}
