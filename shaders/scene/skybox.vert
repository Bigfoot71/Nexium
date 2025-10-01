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

layout(std140, binding = 1) uniform Environment {
    vec3 ambientColor;
    vec4 skyRotation;
    vec3 fogColor;
    vec4 bloomPrefilter;
    float skyIntensity;
    float skySpecular;
    float skyDiffuse;
    float fogDensity;
    float fogStart;
    float fogEnd;
    float fogSkyAffect;
    int fogMode;
    float ssaoIntensity;
    float ssaoRadius;
    float ssaoPower;
    float ssaoBias;
    int ssaoEnabled;
    float bloomFilterRadius;
    float bloomStrength;
    int bloomMode;
    float adjustBrightness;
    float adjustContrast;
    float adjustSaturation;
    float tonemapExposure;
    float tonemapWhite;
    int tonemapMode;
} uEnv;

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
