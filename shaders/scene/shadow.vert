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
#include "../include/frustum.glsl"

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

layout(std140, binding = 0) uniform U_ViewFrustum {
    Frustum uFrustum;
};

layout(std140, binding = 2) uniform U_Renderable {
    mat4 matModel;
    mat4 matNormal;
    int boneOffset;
    uint layerMask;
    bool instancing;
    bool skinning;
} uRender;

layout(std140, binding = 3) uniform U_Material {
    vec4 albedoColor;
    vec3 emissionColor;
    float emissionEnergy;
    float aoLightAffect;
    float occlusion;
    float roughness;
    float metalness;
    float normalScale;
    float alphaCutOff;
    vec2 texOffset;
    vec2 texScale;
    int billboard;
} uMat;

/* === Uniforms === */

layout(location = 0) uniform mat4 uLightViewProj;

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
    mat4 matModel = uRender.matModel;

    if (uRender.skinning) {
        mat4 sMatModel = SkinMatrix(aBoneIDs, aWeights, uRender.boneOffset);
        matModel = sMatModel * matModel;
    }

    if (uRender.instancing) {
        matModel = iMatModel * matModel;
    }

    switch(uMat.billboard) {
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
    vTexCoord = uMat.texOffset + aTexCoord * uMat.texScale;
    vAlpha = aColor.a * iColor.a * uMat.albedoColor.a;

    gl_Position = uLightViewProj * vec4(vPosition, 1.0);
}
