/* scene.vert -- Shared scene vertex shader for all 'lit', 'unlit', 'wireframe', etc, modes.
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
precision mediump int;
#endif

/* === Includes === */

#include "../include/environment.glsl"
#include "../include/billboard.glsl"
#include "../include/frustum.glsl"
#include "../include/frame.glsl"
#include "../include/draw.glsl"

/* === Attributes === */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aTangent;
layout(location = 4) in vec4 aColor;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;
layout(location = 7) in vec3 iPosition;
layout(location = 8) in vec4 iRotation;
layout(location = 9) in vec3 iScale;
layout(location = 10) in vec4 iColor;
layout(location = 11) in vec4 iCustom;

/* === Storage Buffers === */

layout(std430, binding = 0) buffer S_PerModelBuffer {
    ModelData sModelData[];
};

layout(std430, binding = 1) buffer S_PerMeshBuffer {
    MeshData sMeshData[];
};

layout(std430, binding = 2) buffer S_BoneBuffer {
    mat4 sBoneMatrices[];
};

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Frame {
    Frame uFrame;
};

layout(std140, binding = 1) uniform U_ViewFrustum {
    Frustum uFrustum;
};

layout(std140, binding = 2) uniform U_Environment {
    Environment uEnv;
};

/* === Uniforms === */

layout(location = 0) uniform uint uModelDataIndex;
layout(location = 1) uniform uint uMeshDataIndex;

/* === Varyings === */

layout(location = 0) out VaryInternal {
    vec3 position;
    vec2 texCoord;
    vec4 color;
    mat3 tbn;
} vInt;

layout(location = 10) out VaryUser {
    smooth vec4 data4f;
    flat ivec4 data4i;
} vUsr;

/* === Vertex Override === */

#include "../override/scene.vert"

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

    ModelData modelData = sModelData[uModelDataIndex];

    mat4 matModel = modelData.matModel;
    mat3 matNormal = mat3(modelData.matNormal);

    if (modelData.skinning) {
        mat4 sMatModel = SkinMatrix(aBoneIDs, aWeights, modelData.boneOffset);
        matModel = matModel * sMatModel;
        matNormal = matNormal * mat3(transpose(inverse(sMatModel)));
    }

    if (modelData.instancing) {
        mat4 iMatModel = M_TransformToMat4(iPosition, iRotation, iScale);
        matModel = iMatModel * matModel;
        matNormal = mat3(transpose(inverse(iMatModel))) * matNormal;
    }

    switch(sMeshData[uMeshDataIndex].billboard) {
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

    vInt.position = vec3(matModel * vec4(POSITION, 1.0));
    vInt.texCoord = TEXCOORD;
    vInt.color = COLOR;
    vInt.tbn = mat3(T, B, N);

#if defined(SHADOW)
    gl_Position = uFrame.lightViewProj * vec4(vInt.position, 1.0);
#else
    gl_Position = uFrustum.viewProj * vec4(vInt.position, 1.0);
#endif
}
