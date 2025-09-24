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
