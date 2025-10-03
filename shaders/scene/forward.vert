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

layout(std140, binding = 2) uniform U_Renderable {
    mat4 matModel;
    mat4 matNormal;
    int boneOffset;
    uint layerMask;
    bool instancing;
    bool skinning;
    float time;
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
} uMaterial;

/* === Varyings === */

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out vec4 vColor;
layout(location = 3) out mat3 vTBN;

/* === Vertex Override === */

#include "../include/template/scene.vert"

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
    /* --- Calculation of matrices --- */

    mat4 matModel = uRender.matModel;
    mat3 matNormal = mat3(uRender.matNormal);

    if (uRender.skinning) {
        mat4 sMatModel = SkinMatrix(aBoneIDs, aWeights, uRender.boneOffset);
        matModel = sMatModel * matModel;
        matNormal = mat3(transpose(inverse(sMatModel))) * matNormal;
    }

    if (uRender.instancing) {
        matModel = iMatModel * matModel;
        matNormal = mat3(transpose(inverse(iMatModel))) * matNormal;
    }

    switch(uMaterial.billboard) {
    case BILLBOARD_NONE:
        break;
    case BILLBOARD_FRONT:
        BillboardFront(matModel, matNormal, uFrustum.invView);
        break;
    case BILLBOARD_Y_AXIS:
        BillboardYAxis(matModel, matNormal, uFrustum.invView);
        break;
    }

    /* --- Call vertex override and final vertex calculation --- */

    VertexOverride();

    vec3 T = normalize(matNormal * TANGENT.xyz);
    vec3 N = normalize(matNormal * NORMAL);
    vec3 B = normalize(cross(N, T) * TANGENT.w);

    vPosition = vec3(matModel * vec4(POSITION, 1.0));
    vTexCoord = TEXCOORD;
    vColor = COLOR;
    vTBN = mat3(T, B, N);

    gl_Position = uFrustum.viewProj * vec4(vPosition, 1.0);
}
