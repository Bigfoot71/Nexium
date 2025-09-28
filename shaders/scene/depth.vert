/* depth.vert -- Vertex shader used to only render into the depth buffer with alpha cutoff
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

/* === Attributes === */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 4) in vec4 aColor;

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

layout(location = 0) uniform mat4 uMatModel;
layout(location = 1) uniform vec2 uTCOffset;
layout(location = 2) uniform vec2 uTCScale;
layout(location = 3) uniform vec4 uAlpha;

/* === Varyings === */

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out float vAlpha;

/* === Program === */

void main()
{
    vTexCoord = uTCOffset + aTexCoord * uTCScale;
    vAlpha = uAlpha * aColor.a;

    vec4 position = uMatModel * vec4(aPosition, 1.0);
    gl_Position = uFrustum.viewProj * position;
}
