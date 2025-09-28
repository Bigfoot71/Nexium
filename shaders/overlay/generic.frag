/* generic.frag -- Generic fragment shader for overlay rendering
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision mediump float;
#endif

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;

/* === Samplers === */

#if !defined(COLOR)
layout(binding = 0) uniform sampler2D uTexture;
#endif

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Program === */

void main()
{
    FragColor = vColor;

#if defined(TEXTURE)
    FragColor *= texture(uTexture, vTexCoord);
#elif defined(FONT_BITMAP)
    FragColor.a *= texture(uTexture, vTexCoord).r;
#elif defined(FONT_SDF)
    float sdf = texture(uTexture, vTexCoord).r;
    float smoothing = fwidth(sdf);
    FragColor.a *= smoothstep(0.5 - smoothing, 0.5 + smoothing, sdf);
#endif

    FragColor = vec4(FragColor.rgb * FragColor.a, FragColor.a);
}
