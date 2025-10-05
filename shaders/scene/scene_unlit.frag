/* scene_unlit.frag -- Fragment shader for rendering unlit scene
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

#include "../include/environment.glsl"
#include "../include/frustum.glsl"
#include "../include/frame.glsl"
#include "../include/math.glsl"

/* === Varyings === */

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;
layout(location = 3) in mat3 vTBN;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;
layout(binding = 1) uniform sampler2D uTexEmission;
layout(binding = 2) uniform sampler2D uTexORM;
layout(binding = 3) uniform sampler2D uTexNormal;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Frame {
    FrameForward uFrame;
};

layout(std140, binding = 1) uniform U_ViewFrustum {
    Frustum uFrustum;
};

layout(std140, binding = 2) uniform U_Environment {
    Environment uEnv;
};

layout(std140, binding = 3) uniform U_Renderable {
    mat4 matModel;
    mat4 matNormal;
    int boneOffset;
    uint layerMask;
    bool instancing;
    bool skinning;
} uRender;

layout(std140, binding = 4) uniform U_Material {
    vec4 albedoColor;
    vec3 emissionColor;
    float emissionEnergy;
    float aoLightAffect;
    float occlusion;
    float roughness;
    float metalness;
    float normalScale;
    float alphaCutOff;
    vec2 texOffset;
    vec2 texScale;
    int billboard;
} uMaterial;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

/* === Fragment Override === */

#include "../include/template/scene.frag"

/* === Helper functions === */

vec3 NormalScale(vec3 normal, float scale)
{
    normal.xy *= scale;
    normal.z = sqrt(1.0 - clamp(dot(normal.xy, normal.xy), 0.0, 1.0));
    return normal;
}

/* === Program === */

void main()
{
    /* --- Call fragment override --- */

    FragmentOverride();

    /* --- Sample normal and compute view direction vector --- */

    vec3 N = normalize(vTBN * NormalScale(NORMAL_MAP.rgb * 2.0 - 1.0, NORMAL_SCALE));

    /* --- Calculation of the distance from the fragment to the camera in scene units --- */

    float zLinear = F_LinearizeDepth(gl_FragCoord.z, uFrustum.near, uFrustum.far);

    /* --- Calculate and apply fog factor --- */

    float fogFactor = 1.0;

    switch (uEnv.fogMode) {
    case FOG_LINEAR:
        fogFactor = FogLinear(zLinear, uEnv.fogDensity, uEnv.fogStart, uEnv.fogEnd);
        break;
    case FOG_EXP2:
        fogFactor = FogExp2(zLinear, uEnv.fogDensity);
        break;
    case FOG_EXP:
        fogFactor = FogExp(zLinear, uEnv.fogDensity);
        break;
    default:
        break;
    }

    /* --- Fragments outputs --- */

    vec3 color = ALBEDO.rgb + EMISSION.rgb;
    color = mix(uEnv.fogColor, color, fogFactor);

    FragColor = vec4(color, ALBEDO.a);
    FragNormal = vec4(vec2(M_EncodeOctahedral(N)), vec2(1.0));
}
