/* cubemap_from_equirectangular.frag -- Fragment shader for converting equirectangular panorama to cubemap
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
