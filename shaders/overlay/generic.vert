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

/* === Attributes === */

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

/* === Uniforms === */

layout(std140, binding = 0) uniform UniformBlock {
    mat4 uProjection;
};

/* === Varyings === */

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

/* === Program === */

void main()
{
    vTexCoord = aTexCoord;
    vColor = aColor;

    gl_Position = uProjection * vec4(vec3(aPosition, 0.0), 1.0);
}
