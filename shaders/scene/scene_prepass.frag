/* scene_prepass.frag -- Fragment shader for depth pre-pass
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

#include "../include/draw.glsl"

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

/* === Uniforms === */

layout(location = 1) uniform uint uDrawUniqueIndex;

/* === Program === */

void main()
{
    float alpha = vInt.color.a * texture(uTexAlbedo, vInt.texCoord).a;
    if (alpha < sDrawUnique[uDrawUniqueIndex].alphaCutOff) discard;
}
