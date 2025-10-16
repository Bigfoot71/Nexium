/* cubemap_skybox.frag -- Skybox generation fragment shader
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

#include "../include/math.glsl"

/* === Varyings === */

layout(location = 0) in vec3 vPosition;

/* === Uniforms === */

layout(location = 1) uniform vec3 uSunDirection;
layout(location = 2) uniform vec3 uSkyColorTop;
layout(location = 3) uniform vec3 uSkyColorHorizon;
layout(location = 4) uniform vec3 uSunColor;
layout(location = 5) uniform vec3 uGroundColor;
layout(location = 6) uniform float uSunSize;
layout(location = 7) uniform float uHaze;
layout(location = 8) uniform float uEnergy;
layout(location = 9) uniform bool uIsHDR;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Helper Functions === */

vec3 LinearToSRGB(vec3 color)
{
    // color = clamp(color, vec3(0.0), vec3(1.0));
    // const vec3 a = vec3(0.055f);
    // return mix((vec3(1.0f) + a) * pow(color.rgb, vec3(1.0f / 2.4f)) - a, 12.92f * color.rgb, lessThan(color.rgb, vec3(0.0031308f)));
    // Approximation from http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    return max(vec3(1.055) * pow(color, vec3(0.416666667)) - vec3(0.055), vec3(0.0));
}

/* === Program === */

void main()
{
    /* --- Normalization of ray direction --- */

    vec3 rayDir = normalize(vPosition);
    vec3 sunDir = normalize(uSunDirection);

    /* --- Gradient: sky / horizon / ground --- */

    float skyGradient = smoothstep(-0.1, 0.8, asin(rayDir.y));
    vec3 sky = mix(uSkyColorHorizon, uSkyColorTop, skyGradient);

    if (rayDir.y < 0.0) { //< Ground
        float groundFade = smoothstep(-0.05, -0.02, rayDir.y);
        sky = mix(uGroundColor, uSkyColorHorizon, groundFade);
    }

    /* --- Sun and halo --- */

    float sunDist = distance(rayDir, sunDir);
    float sunCore = smoothstep(uSunSize, 0.0, sunDist);                 // sun disk
    float sunHalo = exp(-pow(sunDist / (uSunSize * 2.0), 2.0)) * 0.5;   // soft halo
    vec3 sun = uSunColor * (sunCore + sunHalo);

    sky += sun;

    /* --- Haze / diffusion --- */

    float sunDot = max(dot(rayDir, sunDir), 0.0);
    vec3 haze = mix(vec3(1.0), uSunColor, pow(sunDot, 16.0) * uHaze);
    sky *= haze;

    /* --- HDR: Apply energy and convert to sRGB if needed --- */

    sky *= uEnergy;

    if (!uIsHDR) {
        sky = LinearToSRGB(sky);
    }

    FragColor = vec4(sky, 1.0);
}
