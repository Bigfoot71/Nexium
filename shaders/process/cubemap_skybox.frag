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

/* === Utility Functions === */

vec3 RayleighScattering(vec3 direction, vec3 sunDir)
{
    // Simulates atmospheric Rayleigh scattering (simplified)
    float cosTheta = dot(direction, sunDir);
    float phase = 3.0 / (16.0 * M_PI) * (1.0 + cosTheta * cosTheta);
    return vec3(0.3, 0.6, 1.0) * phase;
}

vec3 MieScattering(vec3 direction, vec3 sunDir, float uHaze)
{
    // Simulates Mie scattering for suspended particles
    float cosTheta = dot(direction, sunDir);
    float g = 0.8; // Anisotropy parameter
    float g2 = g * g;
    float phase = (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5);
    return vec3(1.0, 0.9, 0.7) * phase * uHaze;
}

/* === Program === */

void main()
{
    vec3 rayDir = normalize(vPosition);
    vec3 sunDir = normalize(uSunDirection);

    float rayElevation = asin(rayDir.y);
    float sunElevation = asin(sunDir.y);

    /* --- Sky gradient --- */

    float skyGradient = smoothstep(-0.1, 0.8, rayElevation);
    vec3 skyColor = mix(uSkyColorHorizon, uSkyColorTop, skyGradient);

    /* --- Ground --- */

    vec3 finalColor = skyColor;

    if (rayDir.y < 0.0) {
        float groundFade = smoothstep(-0.05, -0.02, rayDir.y);
        finalColor = mix(uGroundColor, uSkyColorHorizon, groundFade);
    }

    /* --- Sun --- */

    float sunDistance = distance(rayDir, sunDir);
    float sunDisk = 1.0 - smoothstep(uSunSize * 0.8, uSunSize, sunDistance);

    float sunHalo = 1.0 - smoothstep(uSunSize * 2.0, uSunSize * 8.0, sunDistance);
    sunHalo = pow(sunHalo, 3.0) * 0.3;

    float sunCorona = 1.0 - smoothstep(uSunSize * 8.0, uSunSize * 20.0, sunDistance);
    sunCorona = pow(sunCorona, 2.0) * 0.1;

    vec3 sunEffect = uSunColor * (sunDisk + sunHalo + sunCorona);

    /* --- Atmospheric effects --- */

    vec3 rayleigh = RayleighScattering(rayDir, sunDir) * 0.1;
    vec3 mie = MieScattering(rayDir, sunDir, uHaze) * 0.05;

    float dayIntensity = clamp(sunElevation + 0.2, 0.0, 1.0);
    
    /* --- Atmospheric lighting --- */

    float sunInfluence = max(0.0, dot(rayDir, sunDir));
    sunInfluence = pow(sunInfluence, 2.0);

    vec3 atmosphericLight = mix(
        vec3(0.4, 0.5, 0.8),
        uSunColor * 0.3,
        sunInfluence
    );

    /* --- Final assembly --- */

    finalColor += sunEffect;
    finalColor += rayleigh * dayIntensity;
    finalColor += mie * dayIntensity;
    finalColor *= (1.0 + atmosphericLight * 0.2);

    /* --- Final adjustment --- */

    float nightFactor = 1.0 - clamp(sunElevation * 2.0, 0.0, 1.0);
    finalColor = mix(finalColor, finalColor * vec3(0.1, 0.2, 0.4), nightFactor * 0.8);

    // Saturation based on daylight intensity
    float saturation = 0.8 + dayIntensity * 0.4;
    float luminance = dot(finalColor, vec3(0.299, 0.587, 0.114));
    finalColor = mix(vec3(luminance), finalColor, saturation);

    if (uIsHDR) {
        finalColor *= uEnergy;
    }
    else {
        // REVIEW: Can be adapted according to the environment tonemap?
        finalColor = 1.0 - exp(-finalColor * uEnergy);
        finalColor = pow(finalColor, vec3(1.0 / 2.2));
    }

    FragColor = vec4(finalColor, 1.0);
}
