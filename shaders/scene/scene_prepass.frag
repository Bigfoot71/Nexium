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

#include "../include/frame.glsl"
#include "../include/math.glsl"
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
layout(binding = 1) uniform sampler2D uTexEmission;     //< Override compatibility
layout(binding = 2) uniform sampler2D uTexORM;          //< Override compatibility
layout(binding = 3) uniform sampler2D uTexNormal;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Frame {
    Frame uFrame;
};

/* === Uniforms === */

layout(location = 1) uniform uint uDrawUniqueIndex;

/* === Fragments === */

layout(location = 0) out vec4 FragNormal;

/* === Fragment Override === */

#include "../override/scene.frag"

/* === Program === */

void main()
{
    FragmentOverride();

    if (ALBEDO.a < sDrawUnique[uDrawUniqueIndex].alphaCutOff) {
        discard;
    }

    vec3 N = M_NormalScale(NORMAL_MAP.rgb * 2.0 - 1.0, NORMAL_SCALE);
    N = normalize(vInt.tbn * N) * (gl_FrontFacing ? 1.0 : -1.0);
    FragNormal = vec4(M_EncodeOctahedral(N), vec2(1.0));
}
