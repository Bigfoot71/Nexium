/* shadow.vert -- Vertex shader for rendering shadow maps
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

#include "../include/billboard.glsl"

/* === Attributes === */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 4) in vec4 aColor;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;
layout(location = 7) in mat4 iMatModel;
layout(location = 11) in vec4 iColor;

/* === Storage Buffers === */

layout(std430, binding = 0) buffer BoneBuffer {
    mat4 sBoneMatrices[];
};

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

layout(location = 0) uniform mat4 uLightViewProj;
layout(location = 1) uniform mat4 uMatModel;
layout(location = 2) uniform vec2 uTCOffset;
layout(location = 3) uniform vec2 uTCScale;
layout(location = 4) uniform float uAlpha;
layout(location = 5) uniform bool uSkinning;
layout(location = 6) uniform int uBoneOffset;
layout(location = 7) uniform bool uInstancing;
layout(location = 8) uniform uint uBillboard;

/* === Varyings === */

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out float vAlpha;

/* === Helper Functions === */

mat4 SkinMatrix(ivec4 boneIDs, vec4 weights, int offset)
{
    return weights.x * sBoneMatrices[offset + boneIDs.x] +
           weights.y * sBoneMatrices[offset + boneIDs.y] +
           weights.z * sBoneMatrices[offset + boneIDs.z] +
           weights.w * sBoneMatrices[offset + boneIDs.w];
}

/* === Program === */

void main()
{
    mat4 matModel = uMatModel;

    if (uSkinning) {
        mat4 sMatModel = SkinMatrix(aBoneIDs, aWeights, uBoneOffset);
        matModel = sMatModel * matModel;
    }

    if (uInstancing) {
        matModel = iMatModel * matModel;
    }

    switch(uBillboard) {
    case BILLBOARD_NONE:
        break;
    case BILLBOARD_FRONT:
        BillboardFront(matModel, uFrustum.invView);
        break;
    case BILLBOARD_Y_AXIS:
        BillboardYAxis(matModel, uFrustum.invView);
        break;
    }

    vPosition = vec3(matModel * vec4(aPosition, 1.0));
    vTexCoord = uTCOffset + aTexCoord * uTCScale;
    vAlpha = aColor.a * iColor.a * uAlpha;

    gl_Position = uLightViewProj * vec4(vPosition, 1.0);
}
