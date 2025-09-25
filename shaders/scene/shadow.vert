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
layout(location = 4) in vec4 aColor;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;
layout(location = 7) in mat4 iMatModel;
layout(location = 11) in vec4 iColor;

/* === Storage Buffers === */

layout(std430, binding = 0) buffer BoneBuffer {
    mat4 sBoneMatrices[];
};

/* === Uniforms === */

layout(location = 0) uniform mat4 uLightViewProj;
layout(location = 1) uniform mat4 uMatModel;
layout(location = 2) uniform vec2 uTCOffset;
layout(location = 3) uniform vec2 uTCScale;
layout(location = 4) uniform float uAlpha;
layout(location = 5) uniform bool uSkinning;
layout(location = 6) uniform int uBoneOffset;
layout(location = 7) uniform bool uInstancing;

/* === Varyings === */

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec2 vTexCoord;
layout(location = 2) out float vAlpha;

/* === Program === */

void main()
{
    vec3 position = aPosition;

    if (uSkinning)
    {
        mat4 sMatModel =
            aWeights.x * sBoneMatrices[uBoneOffset + aBoneIDs.x] +
            aWeights.y * sBoneMatrices[uBoneOffset + aBoneIDs.y] +
            aWeights.z * sBoneMatrices[uBoneOffset + aBoneIDs.z] +
            aWeights.w * sBoneMatrices[uBoneOffset + aBoneIDs.w];

        position = vec3(sMatModel * vec4(position, 1.0));
    }

    if (uInstancing) {
        position = vec3(iMatModel * vec4(position, 1.0));
    }

    vPosition = vec3(uMatModel * vec4(position, 1.0));
    vTexCoord = uTCOffset + aTexCoord * uTCScale;
    vAlpha = aColor.a * iColor.a * uAlpha;

    gl_Position = uLightViewProj * vec4(vPosition, 1.0);
}
