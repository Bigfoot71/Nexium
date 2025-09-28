/* depth.frag -- Fragment shader used to only render into the depth buffer with alpha cutoff
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

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in float vAlpha;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;

/* === Uniforms === */

layout(location = 4) uniform float uAlphaCutOff;

/* === Program === */

void main()
{
    float alpha = vAlpha * texture(uTexAlbedo, vTexCoord).a;
    if (alpha < uAlphaCutOff) discard;
}
