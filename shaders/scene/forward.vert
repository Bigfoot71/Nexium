/* forward.vert -- Main vertex shader for forward rendering
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
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aTangent;
layout(location = 4) in vec4 aColor;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;
layout(location = 7) in mat4 iMatModel;
layout(location = 11) in vec4 iColor;
layout(location = 12) in vec4 iCustom;

/* === Storage Buffers === */

layout(std430, binding = 4) buffer BoneBuffer {
    mat4 sBoneMatrices[];
};

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_ViewFrustum {
    Frustum uFrustum;
};

layout(std140, binding = 2) uniform U_Material {
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

layout(location = 0) uniform mat4 uMatModel;
layout(location = 1) uniform mat3 uMatNormal;
layout(location = 5) uniform bool uSkinning;
layout(location = 6) uniform int uBoneOffset;
layout(location = 7) uniform bool uInstancing;

/* === Varyings === */

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out vec4 vColor;
layout(location = 3) out mat3 vTBN;

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
    mat3 matNormal = uMatNormal;

    if (uSkinning) {
        mat4 sMatModel = SkinMatrix(aBoneIDs, aWeights, uBoneOffset);
        matModel = sMatModel * matModel;
        matNormal = mat3(transpose(inverse(sMatModel))) * matNormal;
    }

    if (uInstancing) {
        matModel = iMatModel * matModel;
        matNormal = mat3(transpose(inverse(iMatModel))) * matNormal;
    }

    switch(uMat.billboard) {
    case BILLBOARD_NONE:
        break;
    case BILLBOARD_FRONT:
        BillboardFront(matModel, matNormal, uFrustum.invView);
        break;
    case BILLBOARD_Y_AXIS:
        BillboardYAxis(matModel, matNormal, uFrustum.invView);
        break;
    }

    vec3 T = normalize(matNormal * aTangent.xyz);
    vec3 N = normalize(matNormal * aNormal);
    vec3 B = normalize(cross(N, T) * aTangent.w);

    vPosition = vec3(matModel * vec4(aPosition, 1.0));
    vTexCoord = uMat.texOffset + aTexCoord * uMat.texScale;
    vColor = aColor * iColor * uMat.albedoColor;
    vTBN = mat3(T, B, N);

    gl_Position = uFrustum.viewProj * vec4(vPosition, 1.0);
}
