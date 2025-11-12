/* fog.glsl -- Contains everything you need to manage fog
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Constants === */

#define FOG_LINEAR 1
#define FOG_EXP2 2
#define FOG_EXP 3

/* === Structures === */

struct Environment {
    vec3 ambientColor;
    vec4 skyRotation;
    vec3 fogColor;
    vec4 bloomPrefilter;
    float skyIntensity;
    float skySpecular;
    float skyDiffuse;
    int skyProbeIndex;              // -1 if no environment reflections
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
};

/* === Fog Functions === */

float FogLinear(float zLinear, float density, float start, float end)
{
    float fog = clamp((end - zLinear) / (end - start), 0.0, 1.0);
    return mix(1.0 - density, 1.0, fog);
}

float FogExp2(float zLinear, float density)
{
    return exp(-pow(density * zLinear, 2.0));
}

float FogExp(float zLinear, float density)
{
    return exp(-density * zLinear);
}
