/* skybox.frag -- Skybox fragment shader
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

layout(location = 0) in vec3 vPosition;

/* === Samplers === */

layout(binding = 0) uniform samplerCube uTexSkybox;

/* === Uniform Buffers === */

layout(std140, binding = 1) uniform Environment {
    vec3 ambientColor;
    vec4 skyRotation;
    vec3 fogColor;
    vec4 bloomPrefilter;
    float skyIntensity;
    float skySpecular;
    float skyDiffuse;
    float fogDensity;
    float fogStart;
    float fogEnd;
    float fogSkyAffect;
    int fogMode;
    float ssaoIntensity;
    float ssaoRadius;
    float ssaoPower;
    float ssaoBias;
    int ssaoEnabled;
    float bloomFilterRadius;
    float bloomStrength;
    int bloomMode;
    float adjustBrightness;
    float adjustContrast;
    float adjustSaturation;
    float tonemapExposure;
    float tonemapWhite;
    int tonemapMode;
} uEnv;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Programs === */

void main()
{
    vec3 color = texture(uTexSkybox, vPosition).rgb * uEnv.skyIntensity;
    FragColor = vec4(mix(color, uEnv.fogColor, uEnv.fogSkyAffect), 1.0);
}
