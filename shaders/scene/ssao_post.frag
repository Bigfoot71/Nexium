/* ssao_post.frag -- Fragment shader for applying SSAO to the scene
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

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexScene;
layout(binding = 1) uniform sampler2D uTexAO;

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

/* === Program === */

void main()
{
    vec3 scene = texture(uTexScene, vTexCoord).rgb;
    float ao = texture(uTexAO, vTexCoord).r;

    if (uEnv.ssaoPower != 1.0) {
        ao = pow(ao, uEnv.ssaoPower);
    }

    ao = mix(1.0, ao, uEnv.ssaoIntensity);

    FragColor = vec4(scene.rgb * ao, 1.0);
}
