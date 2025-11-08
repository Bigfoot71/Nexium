/* scene_prepass.frag -- Fragment shader for rendering shadow maps
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

#include "../include/lights.glsl"
#include "../include/draw.glsl"

/* === Defines === */

#define SHADOW      //< Definition for frame.glsl

/* === Includes === */

#include "../include/frame.glsl"

/* === Varyings === */

layout(location = 0) in VaryInternal {
    vec3 position;
    vec2 texCoord;
    vec4 color;
    mat3 tbn;
} vInt;

layout(location = 10) in VaryUser {
    smooth vec4 data4f;
    flat ivec4 data4i;
} vUsr;

/* === Storage Buffers === */

layout(std430, binding = 1) buffer S_DrawUniqueBuffer {
    DrawUnique sDrawUnique[];
};

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Frame {
    Frame uFrame;
};

/* === Uniforms === */

layout(location = 1) uniform uint uDrawUniqueIndex;

/* === Fragments === */

layout(location = 0) out vec4 FragDistance;

/* === Program === */

void main()
{
    float alpha = vInt.color.a * texture(uTexAlbedo, vInt.texCoord).a;
    if (alpha < sDrawUnique[uDrawUniqueIndex].alphaCutOff) discard;

    float depth = gl_FragCoord.z;
    if (uFrame.lightType != LIGHT_DIR) {
        depth = length(vInt.position - uFrame.lightPosition) / uFrame.lightRange;
    }

    FragDistance = vec4(depth, 0.0, 0.0, 1.0);
}
