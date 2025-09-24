/*
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

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexEquirectangular;

/* === Uniforms === */

layout(location = 0) uniform int uFace;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Helper Functions === */

vec3 GetDirection(int face, vec2 uv)
{
    // uv * 2.0 - 1.0 : [0,1] -> [-1,1]
    uv = fma(uv, vec2(2.0), vec2(-1.0));
    switch(face) {
    case 0: return normalize(vec3( 1.0, -uv.y, -uv.x)); // +X
    case 1: return normalize(vec3(-1.0, -uv.y,  uv.x)); // -X
    case 2: return normalize(vec3( uv.x,  1.0,  uv.y)); // +Y
    case 3: return normalize(vec3( uv.x, -1.0, -uv.y)); // -Y
    case 4: return normalize(vec3( uv.x, -uv.y,  1.0)); // +Z
    case 5: return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z
    }
    return vec3(0.0);
}

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, -0.3183);
    uv += 0.5;
    return uv;
}

/* === Program === */

void main()
{
    vec3 dir = GetDirection(uFace, vTexCoord);
    FragColor = texture(uTexEquirectangular, SampleSphericalMap(dir));
}
