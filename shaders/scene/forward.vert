/**
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

/* === Attributes === */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aTangent;
layout(location = 4) in vec4 aColor;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;

/* === Storage Buffers === */

layout(std430, binding = 4) buffer BoneBuffer {
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

layout(location = 0) uniform mat4 uMatModel;
layout(location = 1) uniform mat3 uMatNormal;

layout(location = 2) uniform vec4 uColAlbedo;
layout(location = 3) uniform vec2 uTCOffset;
layout(location = 4) uniform vec2 uTCScale;

layout(location = 5) uniform bool uSkinning;
layout(location = 6) uniform int uBoneOffset;

/* === Varyings === */

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out vec4 vColor;
layout(location = 3) out mat3 vTBN;

/* === Program === */

void main()
{
    vec3 skinPosition = aPosition;
    vec3 skinTangent = aTangent.xyz;
    vec3 skinNormal = aNormal;

    if (uSkinning)
    {
        mat4 skinMatModel =
            aWeights.x * sBoneMatrices[uBoneOffset + aBoneIDs.x] +
            aWeights.y * sBoneMatrices[uBoneOffset + aBoneIDs.y] +
            aWeights.z * sBoneMatrices[uBoneOffset + aBoneIDs.z] +
            aWeights.w * sBoneMatrices[uBoneOffset + aBoneIDs.w];

        mat3 skinMatNormal = mat3(transpose(inverse(skinMatModel)));

        skinPosition = vec3(skinMatModel * vec4(aPosition, 1.0));
        skinTangent = skinMatNormal * aTangent.xyz;
        skinNormal = skinMatNormal * aNormal;
    }

    vec3 T = normalize(uMatNormal * skinTangent);
    vec3 N = normalize(uMatNormal * skinNormal);
    vec3 B = normalize(cross(N, T) * aTangent.w);

    vPosition = vec3(uMatModel * vec4(skinPosition, 1.0));
    vTexCoord = uTCOffset + aTexCoord * uTCScale;
    vColor = aColor * uColAlbedo;
    vTBN = mat3(T, B, N);

    gl_Position = uFrustum.viewProj * vec4(vPosition, 1.0);
}
