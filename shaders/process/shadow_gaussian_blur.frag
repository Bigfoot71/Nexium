/* shadow_gaussian_blur.frag -- Gaussian blur (two passes) fragment shader for shadow maps
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

// NOTE: The coefficients for the two-pass Gaussian blur were generated using:
//       https://lisyarus.github.io/blog/posts/blur-coefficients-generator.html

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
#endif

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

#if defined(FIRST_PASS_CUBE)
layout(binding = 0) uniform highp samplerCubeArray uTexShadow;
#elif defined(FIRST_PASS_2D)
layout(binding = 0) uniform highp sampler2DArray uTexShadow;
#else
layout(binding = 0) uniform sampler2D uTexShadow;
#endif

/* === Uniforms === */

#if defined(FIRST_PASS_CUBE) || defined(FIRST_PASS_2D)
layout(location = 0) uniform int uShadowMapIndex;
#endif

#if defined(FIRST_PASS_CUBE)
layout(location = 1) uniform int uCubeFace;
#endif

layout(location = 2) uniform float uSoftness;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Blur Coefs === */

const int SAMPLE_COUNT = 6;

const float OFFSETS[6] = float[6](
    -4.268941421369995,
    -2.364576440741639,
    -0.4722507649454868,
    1.4174297935376854,
    3.3147990233346842,
    5
);

const float WEIGHTS[6] = float[6](
    0.043867558300718715,
    0.1914659874907833,
    0.3595399106052396,
    0.2914549970600666,
    0.1018737430617653,
    0.011797803481426415
);

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

vec2 SampleShadow(vec2 texCoord)
{
    vec2 result;

#if defined(FIRST_PASS_CUBE)
    vec3 direction = GetDirection(uCubeFace, texCoord);
    result = texture(uTexShadow, vec4(direction, float(uShadowMapIndex))).rg;
#elif defined(FIRST_PASS_2D)
    result = texture(uTexShadow, vec3(texCoord, float(uShadowMapIndex))).rg;
#else
    result = texture(uTexShadow, texCoord).rg;
#endif

    return result;
}

/* === Main Program === */

void main()
{
    vec2 result = vec2(0.0);

    float texelSize = 1.0 / float(textureSize(uTexShadow, 0).x);
    float blurRadius = uSoftness * texelSize;

#if defined(FIRST_PASS_CUBE) || defined(FIRST_PASS_2D)
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        result += SampleShadow(vTexCoord + vec2(blurRadius *  OFFSETS[i], 0.0)) * WEIGHTS[i];
    }
#else
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        result += SampleShadow(vTexCoord + vec2(0.0, blurRadius *  OFFSETS[i])) * WEIGHTS[i];
    }
#endif

    FragColor = vec4(vec2(result), vec2(1.0));
}
