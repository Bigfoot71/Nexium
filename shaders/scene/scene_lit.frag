/* scene_lit.frag -- Fragment shader for rendering lit scene
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
precision mediump int;
#endif

/* === Includes === */

#include "../include/environment.glsl"
#include "../include/frustum.glsl"
#include "../include/lights.glsl"
#include "../include/frame.glsl"
#include "../include/draw.glsl"
#include "../include/math.glsl"
#include "../include/pbr.glsl"

/* === Constants === */

#define SHADOW_SAMPLES 16

const vec2 POISSON_DISK[16] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790)
);

/* === Varyings === */

layout(location = 0) in VaryInternal {
    vec3 position;
    vec2 texCoord;
    vec4 color;
    mat3 tbn;
} vInt;

layout(location = 10) in VaryUser {
    smooth vec4 data4f;
    flat ivec4 data4i;
} vUsr;

/* === Storage Buffers === */

layout(std430, binding = 1) buffer S_PerMeshBuffer {
    MeshData sMeshData[];
};

layout(std430, binding = 3) buffer S_LightBuffer {
    Light sLights[];
};

layout(std430, binding = 4) buffer S_ShadowBuffer {
    Shadow sShadows[];
};

layout(std430, binding = 5) buffer S_ClusterBuffer {
    uint sClusters[]; //< Contains number of lights for each tile
};

layout(std430, binding = 6) buffer S_IndexBuffer {
    uint sIndices[]; //< Contains the light indices for each tile, in increments of 'uMaxLightsPerTile'
};

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;
layout(binding = 1) uniform sampler2D uTexEmission;
layout(binding = 2) uniform sampler2D uTexORM;
layout(binding = 3) uniform sampler2D uTexNormal;

layout(binding = 4) uniform sampler2D uTexBrdfLut;
layout(binding = 5) uniform samplerCube uTexProbeIrradiance;
layout(binding = 6) uniform samplerCube uTexProbePrefilter;

layout(binding = 7) uniform highp sampler2DArray uTexShadowDir;
layout(binding = 8) uniform highp sampler2DArray uTexShadowSpot;
layout(binding = 9) uniform highp samplerCubeArray uTexShadowOmni;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform U_Frame {
    Frame uFrame;
};

layout(std140, binding = 1) uniform U_ViewFrustum {
    Frustum uFrustum;
};

layout(std140, binding = 2) uniform U_Environment {
    Environment uEnv;
};

/* === Uniforms === */

layout(location = 1) uniform uint uMeshDataIndex;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

/* === Fragment Override === */

#include "../override/scene.frag"

/* === Lighting Functions === */

float Diffuse(float cLdotH, float cNdotV, float cNdotL, float roughness)
{
    float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
    float FdV = 1.0 + FD90_minus_1 * PBR_SchlickFresnel(cNdotV);
    float FdL = 1.0 + FD90_minus_1 * PBR_SchlickFresnel(cNdotL);

    return (1.0 / M_PI) * (FdV * FdL * cNdotL); // Diffuse BRDF (Burley)
}

vec3 Specular(vec3 F0, float cLdotH, float cNdotH, float cNdotV, float cNdotL, float alphaGGX)
{
    float D = PBR_DistributionGGX(cNdotH, alphaGGX);
    float G = PBR_GeometryGGX(cNdotL, cNdotV, alphaGGX);

    float cLdotH5 = PBR_SchlickFresnel(cLdotH);
    float F90 = clamp(50.0 * F0.g, 0.0, 1.0);
    vec3 F = F0 + (F90 - F0) * cLdotH5;

    return cNdotL * D * F * G; // Specular BRDF (Schlick GGX)
}

/* === Shadow functions === */

float InterleavedGradientNoise(vec2 pos)
{
    // http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
	const vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	return fract(magic.z * fract(dot(pos, magic.xy)));
}

float ShadowDir(in Light light, float NdotL)
{
    Shadow shadow = sShadows[light.shadowIndex];

    /* --- Light space projection --- */

    vec4 projPos = shadow.viewProj * vec4(vInt.position, 1.0);
    vec3 projCoords = projPos.xyz / projPos.w * 0.5 + 0.5;

    /* --- Calculate current normalized depth and apply bias --- */

    vec3 lightToFrag = vInt.position - light.position;
    float lightToFragLen = length(lightToFrag);

    float bias = max(shadow.bias, shadow.slopeBias * (1.0 - NdotL));
    float currentDepth = lightToFragLen / light.range - bias;

    /* --- Generate an additional debanding rotation for the poisson disk --- */

    float r = M_TAU * InterleavedGradientNoise(gl_FragCoord.xy);
    float sr = sin(r), cr = cos(r);

    mat2 diskRot = mat2(vec2(cr, -sr), vec2(sr, cr));

    /* --- Poisson Disk PCF Sampling --- */

    float softRadius = shadow.softness / float(textureSize(uTexShadowSpot, 0).x);

    float factor = 0.0;
    for (int j = 0; j < SHADOW_SAMPLES; ++j) {
        vec2 sampleDir = projCoords.xy + diskRot * POISSON_DISK[j] * softRadius;
        factor += step(currentDepth, texture(uTexShadowDir, vec3(sampleDir, float(shadow.mapIndex))).r);
    }

    factor /= float(SHADOW_SAMPLES);

    /* --- Edge Fading --- */

    vec3 distToBorder = min(projCoords, 1.0 - projCoords);
    float minDist = min(distToBorder.x, min(distToBorder.y, distToBorder.z));

    const float fadeStart = 0.85;
    float edgeFade = smoothstep(0.0, 1.0 - fadeStart, minDist);

    /* --- Final Shadow Value --- */

    return mix(1.0, factor, edgeFade);
}

float ShadowSpot(in Light light, float NdotL)
{
    Shadow shadow = sShadows[light.shadowIndex];

    /* --- Light space projection --- */

    vec4 projPos = shadow.viewProj * vec4(vInt.position, 1.0);
    vec3 projCoords = projPos.xyz / projPos.w * 0.5 + 0.5;

    /* --- Shadow map bounds check --- */

    if (any(greaterThan(projCoords.xyz, vec3(1.0))) ||
        any(lessThan(projCoords.xyz, vec3(0.0)))) {
        return 1.0;
    }

    /* --- Calculate current normalized depth and apply bias --- */

    vec3 lightToFrag = vInt.position - light.position;
    float lightToFragLen = length(lightToFrag);

    float bias = max(shadow.bias, shadow.slopeBias * (1.0 - NdotL));
    float currentDepth = lightToFragLen / light.range - bias;

    /* --- Generate an additional debanding rotation for the poisson disk --- */

    float r = M_TAU * InterleavedGradientNoise(gl_FragCoord.xy);
    float sr = sin(r), cr = cos(r);

    mat2 diskRot = mat2(vec2(cr, -sr), vec2(sr, cr));

    /* --- Poisson Disk PCF Sampling --- */

    float softRadius = shadow.softness / float(textureSize(uTexShadowSpot, 0).x);

    float factor = 0.0;
    for (int j = 0; j < SHADOW_SAMPLES; ++j) {
        vec2 sampleDir = projCoords.xy + diskRot * POISSON_DISK[j] * softRadius;
        factor += step(currentDepth, texture(uTexShadowSpot, vec3(sampleDir, float(shadow.mapIndex))).r);
    }

    /* --- Final Shadow Value --- */

    return factor / float(SHADOW_SAMPLES);
}

float ShadowOmni(in Light light, float NdotL)
{
    Shadow shadow = sShadows[light.shadowIndex];

    /* --- Calculate light vector --- */

    vec3 lightToFrag = vInt.position - light.position;
    float lightToFragLen = length(lightToFrag);
    vec3 dir = lightToFrag / lightToFragLen;

    /* --- Calculate current normalized depth and apply bias --- */

    float bias = max(shadow.bias, shadow.slopeBias * (1.0 - NdotL));
    float currentDepth = lightToFragLen / light.range - bias;

    /* --- Build orthonormal basis for perturbation --- */

    mat3 OBN = M_OrthonormalBasis(dir);

    /* --- Generate an additional debanding rotation for the poisson disk --- */

    float r = M_TAU * InterleavedGradientNoise(gl_FragCoord.xy);
    float sr = sin(r), cr = cos(r);

    mat2 diskRot = mat2(vec2(cr, -sr), vec2(sr, cr));

    /* --- Poisson Disk PCF Sampling --- */

    float softRadius = shadow.softness / float(textureSize(uTexShadowSpot, 0).x);

    float factor = 0.0;
    for (int j = 0; j < SHADOW_SAMPLES; ++j) {
        vec3 sampleDir = normalize(dir + OBN * vec3(diskRot * POISSON_DISK[j] * softRadius, 0.0));
        float sampleDepth = texture(uTexShadowOmni, vec4(sampleDir, float(shadow.mapIndex))).r;
        factor += step(currentDepth, sampleDepth);
    }

    /* --- Final Shadow Value --- */

    return factor / float(SHADOW_SAMPLES);
}

/* === IBL Functions === */

vec3 IBL_FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    // TODO: See approximations, but this version seems to introduce less bias for grazing angles
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float IBL_GetSpecularOcclusion(float NdotV, float ao, float roughness)
{
    // Lagarde method: https://seblagarde.wordpress.com/wp-content/uploads/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
    return clamp(pow(NdotV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

vec3 IBL_GetMultiScatterBRDF(float NdotV, float roughness, vec3 F0, float metalness)
{
    // Adapted from: https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf
    // TODO: Maybe need a review

    vec2 brdf = texture(uTexBrdfLut, vec2(NdotV, roughness)).rg;

    // Energy compensation for multiple scattering
    vec3 FssEss = F0 * brdf.x + brdf.y;
    float Ess = brdf.x + brdf.y;
    float Ems = 1.0 - Ess;

    // Calculation of Favg adapted to metalness
    // For dielectrics: classical approximation
    // For metals: direct use of F0
    vec3 Favg = mix(
        F0 + (1.0 - F0) / 21.0,  // Dielectric: approximation of the Fresnel integral
        F0,                      // Metal: F0 already colored and raised
        metalness
    );

    // Adapted energy compensation
    vec3 Fms = FssEss * Favg / (1.0 - Favg * Ems + 1e-5); // +epsilon to avoid division by 0

    // For metals, slightly reduce the multiple scattering
    // effect as they absorb more energy with each bounce
    float msStrength = mix(1.0, 0.8, metalness);

    return FssEss + Fms * Ems * msStrength;
}

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

    /* --- Pre-calculation of ORM related data --- */ 

    float alphaGGX = max(ROUGHNESS * ROUGHNESS, 1e-6);
    float oneMinusMetalness = 1.0 - METALNESS;

    /* --- Calculation of the distance from the fragment to the camera in scene units --- */

    float zLinear = F_LinearizeDepth(gl_FragCoord.z, uFrustum.near, uFrustum.far);

    /* --- Compute F0 (reflectance at normal incidence) based on the metallic factor --- */

    vec3 F0 = PBR_ComputeF0(METALNESS, 0.5, ALBEDO.rgb);

    /* --- Sample normal and compute view direction vector --- */

    vec3 N = normalize(vInt.tbn * NormalScale(NORMAL_MAP.rgb * 2.0 - 1.0, NORMAL_SCALE));
    vec3 V = normalize(uFrustum.position - vInt.position);

    /* --- Compute the dot product of the normal and view direction --- */

    float NdotV = dot(N, V);
    float cNdotV = max(NdotV, 1e-4); // Clamped to avoid division by zero

    /* --- Getting the cluster index and the number of lights in the cluster --- */

    uvec3 clusterCoord = L_ClusterFromScreen(gl_FragCoord.xy / vec2(uFrame.screenSize),
        -zLinear, uFrame.clusterCount, uFrame.clusterSliceScale, uFrame.clusterSliceBias);

    uint clusterIndex = L_ClusterIndex(clusterCoord, uFrame.clusterCount);
    uint lightCount = sClusters[clusterIndex];

    // Mask light count if there are no active lights,
    // sClusters may contain stale/garbage values otherwise.
    lightCount *= uint(uFrame.hasActiveLights);

    /* --- Loop through all light sources accumulating diffuse and specular light --- */

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (uint i = 0u; i < lightCount; i++)
    {
        /* --- Calculate light index and get light --- */

        uint lightIndex = sIndices[clusterIndex * uFrame.maxLightsPerCluster + i];
        Light light = sLights[lightIndex];

        /* --- Compute light direction --- */

        vec3 L = (light.type != LIGHT_DIR)
            ? normalize(light.position - vInt.position)
            : -light.direction;

        /* --- Compute the dot product of the normal and light direction --- */

        float NdotL = dot(N, L);
        if (NdotL <= 0.0) continue;
        float cNdotL = min(NdotL, 1.0); // clamped NdotL

        /* --- Compute the halfway vector between the view and light directions --- */

        vec3 H = normalize(V + L);

        float LdotH = max(dot(L, H), 0.0);
        float cLdotH = min(LdotH, 1.0);

        float NdotH = max(dot(N, H), 0.0);
        float cNdotH = min(NdotH, 1.0);

        /* --- Compute light color energy --- */

        vec3 lightColE = light.color * light.energy;

        /* --- Compute diffuse lighting --- */

        vec3 diffLight = vec3(Diffuse(cLdotH, cNdotV, cNdotL, ROUGHNESS));
        diffLight *= lightColE * oneMinusMetalness; // 0.0 for pure metal, 1.0 for dielectric

        /* --- Compute specular lighting --- */

        vec3 specLight = Specular(F0, cLdotH, cNdotH, cNdotV, cNdotL, alphaGGX);
        specLight *= lightColE * light.specular;

        /* --- Apply shadow factor if the light casts shadows --- */

        float shadow = 1.0;
        if (light.shadowIndex >= 0) {
            switch (light.type) {
            case LIGHT_DIR:
                shadow = ShadowDir(light, cNdotL);
                break;
            case LIGHT_SPOT:
                shadow = ShadowSpot(light, cNdotL);
                break;
            case LIGHT_OMNI:
                shadow = ShadowOmni(light, cNdotL);
                break;
            }
        }

        /* --- Apply attenuation based on the distance from the light --- */

        if (light.type != LIGHT_DIR) {
            float dist = length(light.position - vInt.position);
            float atten = 1.0 - clamp(dist / light.range, 0.0, 1.0);
            shadow *= atten * light.attenuation;
        }

        /* --- Apply spotlight effect if the light is a spotlight --- */

        if (light.type == LIGHT_SPOT) {
            float theta = dot(L, -light.direction);
            float epsilon = (light.innerCutOff - light.outerCutOff);
            shadow *= smoothstep(0.0, 1.0, (theta - light.outerCutOff) / epsilon);
        }

        /* --- Accumulate the diffuse and specular lighting contributions --- */

        bool validLayer = ((light.cullMask & sMeshData[uMeshDataIndex].layerMask) != 0u);
        float contribution = shadow * float(validLayer);

        diffuse += diffLight * contribution;
        specular += specLight * contribution;
    }

    /* --- Compute AO light affect --- */

    diffuse *= mix(1.0, OCCLUSION, AO_LIGHT_AFFECT);

    /* --- Ambient diffuse from sky --- */

    vec3 skyDiffuse = uEnv.ambientColor;

    if (uFrame.hasProbe) {
        vec3 Nr = M_Rotate3D(N, uEnv.skyRotation);
        skyDiffuse = texture(uTexProbeIrradiance, Nr).rgb;
    }

    vec3 kS = IBL_FresnelSchlickRoughness(cNdotV, F0, ROUGHNESS);
    vec3 kD = (1.0 - kS) * oneMinusMetalness;

    skyDiffuse *= kD * uEnv.skyDiffuse * OCCLUSION;

    /* --- Ambient specular from sky --- */

    vec3 skySpecular = uEnv.ambientColor;

    if (uFrame.hasProbe) {
        vec3 R = M_Rotate3D(reflect(-V, N), uEnv.skyRotation);
        float mipLevel = ROUGHNESS * (float(textureQueryLevels(uTexProbePrefilter)) - 1.0);
        skySpecular = textureLod(uTexProbePrefilter, R, mipLevel).rgb;
    }

    // Applies fog according to skyAffect to the source prefilter or ambient color
    skySpecular = mix(skySpecular, uEnv.fogColor, uEnv.fogSkyAffect);

    float specOcclusion = IBL_GetSpecularOcclusion(cNdotV, OCCLUSION, ROUGHNESS);
    vec3 specBRDF = IBL_GetMultiScatterBRDF(cNdotV, ROUGHNESS, F0, METALNESS);
    skySpecular *= specBRDF * uEnv.skySpecular * specOcclusion;

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

    /* --- Compute the final fragment color by combining albedo, lighting contributions and fog --- */

    vec3 litColor = ALBEDO.rgb * (skyDiffuse + diffuse);
    litColor += skySpecular + specular;
    litColor += EMISSION;

    FragColor.rgb = mix(uEnv.fogColor, litColor, fogFactor);
    FragColor.a   = ALBEDO.a;

    /* --- Store normals --- */

    FragNormal = vec4(vec2(M_EncodeOctahedral(N)), vec2(1.0));

    /* DEBUG: Tiles */

    //FragColor = (lightCount == 0u) ? vec4(0.0) : vec4(1.0);
}
