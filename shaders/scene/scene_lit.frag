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

#ifndef SHADOW_SAMPLES
#define SHADOW_SAMPLES 8
#endif

#if SHADOW_SAMPLES == 4
const vec2 VOGEL_DISK[4] = vec2[4](
    vec2(0.353553, 0.000000),
    vec2(-0.451544, 0.413652),
    vec2(0.069116, -0.787542),
    vec2(0.569142, 0.742346)
);
#elif SHADOW_SAMPLES == 8
const vec2 VOGEL_DISK[8] = vec2[8](
    vec2(0.250000, 0.000000),
    vec2(-0.319290, 0.292496),
    vec2(0.048872, -0.556877),
    vec2(0.402444, 0.524918),
    vec2(-0.738535, -0.130636),
    vec2(0.699605, -0.445031),
    vec2(-0.234004, 0.870484),
    vec2(-0.446271, -0.859268)
);
#elif SHADOW_SAMPLES == 16
const vec2 VOGEL_DISK[16] = vec2[16](
    vec2(0.176777, 0.000000),
    vec2(-0.225772, 0.206826),
    vec2(0.034558, -0.393771),
    vec2(0.284571, 0.371173),
    vec2(-0.522223, -0.092374),
    vec2(0.494695, -0.314685),
    vec2(-0.165466, 0.615525),
    vec2(-0.315561, -0.607594),
    vec2(0.684642, 0.250030),
    vec2(-0.712256, 0.294009),
    vec2(0.343354, -0.733729),
    vec2(0.253730, 0.808932),
    vec2(-0.764746, -0.443186),
    vec2(0.897134, -0.197232),
    vec2(-0.547507, 0.778772),
    vec2(-0.126487, -0.976090)
);
#endif

/* === Structures === */

struct ReflectionProbe {
    vec3 position;
    float range;
    float falloff;
    uint index;
};

struct LightParams {
    vec3 N, V, F0;
    vec3 diffuse;
    float cNdotV;
    float alphaGGX;
    mat2  diskRotation;
};

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

/** 
 * sDrawUnique[] : per-draw-call data
 *   - Contains unique draw-call parameters
 *   - One entry per rendered object
 */
layout(std430, binding = 1) buffer S_DrawUniqueBuffer {
    DrawUnique sDrawUnique[];
};

/** 
 * sReflectionProbes[] : list of active reflection probes
 *   - Contains per-probe parameters (position, range, falloff, index)
 *   - Used for blending local and global IBL contributions
 */
layout(std430, binding = 3) buffer S_ProbeBuffer {
    ReflectionProbe sReflectionProbes[];
};

/** 
 * sLights[] : list of active lights
 *   - MUST be sorted CPU-side: DIR -> SPOT -> OMNI
 *   - Shaders assume this order for sIndices and per-type offsets
 */
layout(std430, binding = 4) buffer S_LightBuffer {
    Light sLights[];
};

/** 
 * sShadows[] : list of lights casting shadows
 *   - Indexed via Light.shadowIndex when shadow casting is active
 *   - Contains shadow map parameters per light
 */
layout(std430, binding = 5) buffer S_ShadowBuffer {
    Shadow sShadows[];
};

/** 
 * sClusters[] : per-cluster info
 *   - xyz = number of lights per type (numDir, numSpot, numOmni)
 *   - w   = unused
 */
layout(std430, binding = 6) buffer S_ClusterBuffer {
    uvec4 sClusters[];
};

/** 
 * sIndices[] : light indices per cluster
 *   - Indices into sLights[], grouped by type: DIR -> SPOT -> OMNI
 */
layout(std430, binding = 7) buffer S_IndexBuffer {
    uint sIndices[];
};

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexAlbedo;
layout(binding = 1) uniform sampler2D uTexEmission;
layout(binding = 2) uniform sampler2D uTexORM;
layout(binding = 3) uniform sampler2D uTexNormal;

layout(binding = 4) uniform sampler2D uTexBrdfLut;
layout(binding = 5) uniform highp samplerCubeArray uTexIrradiance;
layout(binding = 6) uniform highp samplerCubeArray uTexPrefilter;

layout(binding = 7) uniform mediump sampler2DArray uTexShadowDir;
layout(binding = 8) uniform mediump sampler2DArray uTexShadowSpot;
layout(binding = 9) uniform mediump samplerCubeArray uTexShadowOmni;

#if defined(PREPASS)
layout(binding = 10) uniform sampler2D uTexNormalBuffer;
layout(binding = 11) uniform sampler2D uTexSsaoBuffer;
#endif

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

layout(location = 1) uniform uint uDrawUniqueIndex;

#if defined(PREPASS)
layout(location = 2) uniform vec2 uInvResolution;
#endif

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Fragment Override === */

#include "../override/scene.frag"

/* === PBR Lighting Functions === */

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

/* === Light Functions === */

vec3 LightDir(uint lightIndex, const in LightParams params)
{
    /* --- Checking the layer mask for lighting --- */

    if ((sLights[lightIndex].cullMask & sDrawUnique[uDrawUniqueIndex].layerMask) == 0u) {
        return vec3(0.0);
    }

    Light light = sLights[lightIndex];

    /* --- Compute light and halfway vectors --- */

    vec3 L = -light.direction;

    float NdotL = dot(params.N, L);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 H = normalize(params.V + L);
    float NdotH = max(dot(params.N, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    /* --- Compute light contribution --- */

    vec3 diff = params.diffuse * Diffuse(LdotH, params.cNdotV, NdotL, ROUGHNESS);
    vec3 spec = Specular(params.F0, LdotH, NdotH, params.cNdotV, NdotL, params.alphaGGX);

    vec3 Lo = (diff + spec) * light.color * light.energy;

    /* --- Compute shadow attenuation --- */

    if (light.shadowIndex >= 0)
    {
        Shadow shadow = sShadows[light.shadowIndex];

        /* --- Light space projection --- */

        vec4 projPos = shadow.viewProj * vec4(vInt.position, 1.0);
        vec3 projCoords = projPos.xyz / projPos.w * 0.5 + 0.5;

        /* --- Get current normalized depth with bias --- */

        float bias = max(shadow.bias, shadow.slopeBias * (1.0 - NdotL));
        float currentDepth = projCoords.z - bias;

        /* --- Vogel disk PCF sampling --- */

        float softRadius = shadow.softness / float(textureSize(uTexShadowDir, 0).x);

        float shadowAtten = 0.0;
        for (int i = 0; i < SHADOW_SAMPLES; ++i) {
            vec2 sampleDir = projCoords.xy + params.diskRotation * VOGEL_DISK[i] * softRadius;
            shadowAtten += step(currentDepth, texture(uTexShadowDir, vec3(sampleDir, float(shadow.mapIndex))).r);
        }

        shadowAtten = mix(1.0, shadowAtten / float(SHADOW_SAMPLES), shadow.opacity);

        /* --- Applying a fade to the edges of the projection --- */

        vec3 distToBorder = min(projCoords, 1.0 - projCoords);
        float edgeFade = smoothstep(0.0, 0.15, min(distToBorder.x, min(distToBorder.y, distToBorder.z)));

        Lo *= mix(1.0, shadowAtten, edgeFade);
    }

    /* --- Return final contribution --- */

    return Lo;
}

vec3 LightSpot(uint lightIndex, const in LightParams params)
{
    /* --- Checking the layer mask for lighting --- */

    if ((sLights[lightIndex].cullMask & sDrawUnique[uDrawUniqueIndex].layerMask) == 0u) {
        return vec3(0.0);
    }

    Light light = sLights[lightIndex];

    /* --- Compute frag to light vector and distance --- */

    vec3 toLight = light.position - vInt.position;
    float toLightDist = length(toLight);

    float toLightDist01 = toLightDist / light.range;
    if (toLightDist01 > 1.0) return vec3(0.0);

    /* --- Compute light and halfway vectors --- */

    vec3 L = toLight / toLightDist; // normalize

    float NdotL = dot(params.N, L);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 H = normalize(params.V + L);
    float NdotH = max(dot(params.N, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    /* --- Compute diffuse and specular contribution --- */

    vec3 diff = params.diffuse * Diffuse(LdotH, params.cNdotV, NdotL, ROUGHNESS);
    vec3 spec = Specular(params.F0, LdotH, NdotH, params.cNdotV, NdotL, params.alphaGGX);

    vec3 Lo = (diff + spec) * light.color * light.energy;

    /* --- Compute distance and spotlight attenuation --- */

    float attenuation = pow(1.0 - toLightDist01, light.attenuation);

    float theta = dot(L, -light.direction);
    float epsilon = (light.innerCutOff - light.outerCutOff);
    attenuation *= smoothstep(0.0, 1.0, (theta - light.outerCutOff) / epsilon);

    /* --- Compute shadow attenuation --- */

    if (light.shadowIndex >= 0)
    {
        Shadow shadow = sShadows[light.shadowIndex];

        /* --- Light space projection --- */

        vec4 projPos = shadow.viewProj * vec4(vInt.position, 1.0);
        vec3 projCoords = projPos.xyz / projPos.w * 0.5 + 0.5;

        /* --- Get current normalized depth with bias --- */

        float bias = max(shadow.bias, shadow.slopeBias * (1.0 - NdotL));
        float currentDepth = toLightDist01 - bias;

        /* --- Vogel disk PCF sampling --- */

        float softRadius = shadow.softness / float(textureSize(uTexShadowSpot, 0).x);

        float shadowAtten = 0.0;
        for (int i = 0; i < SHADOW_SAMPLES; ++i) {
            vec2 sampleDir = projCoords.xy + params.diskRotation * VOGEL_DISK[i] * softRadius;
            shadowAtten += step(currentDepth, texture(uTexShadowSpot, vec3(sampleDir, float(shadow.mapIndex))).r);
        }

        shadowAtten = mix(1.0, shadowAtten / float(SHADOW_SAMPLES), shadow.opacity);

        /* --- Apply shadow attenuation with out of bounds mask --- */

        bool outOfBounds = any(lessThan(projCoords, vec3(0.0))) ||
                           any(greaterThan(projCoords, vec3(1.0)));

        attenuation *= mix(shadowAtten, 1.0, float(outOfBounds));
    }

    /* --- Return final contribution --- */

    return Lo * attenuation;
}

vec3 LightOmni(uint lightIndex, const in LightParams params)
{
    /* --- Checking the layer mask for lighting --- */

    if ((sLights[lightIndex].cullMask & sDrawUnique[uDrawUniqueIndex].layerMask) == 0u) {
        return vec3(0.0);
    }

    Light light = sLights[lightIndex];

    /* --- Compute frag to light vector and distance --- */

    vec3 toLight = light.position - vInt.position;
    float toLightDist = length(toLight);

    float toLightDist01 = toLightDist / light.range;
    if (toLightDist01 > 1.0) return vec3(0.0);

    /* --- Compute light and halfway vectors --- */

    vec3 L = toLight / toLightDist; // normalize

    float NdotL = dot(params.N, L);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 H = normalize(params.V + L);
    float NdotH = max(dot(params.N, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    /* --- Compute diffuse and specular contribution --- */

    vec3 diff = params.diffuse * Diffuse(LdotH, params.cNdotV, NdotL, ROUGHNESS);
    vec3 spec = Specular(params.F0, LdotH, NdotH, params.cNdotV, NdotL, params.alphaGGX);

    vec3 Lo = (diff + spec) * light.color * light.energy;

    /* --- Compute distance attenuation --- */

    float attenuation = pow(1.0 - toLightDist01, light.attenuation);

    /* --- Compute shadow attenuation --- */

    if (light.shadowIndex >= 0)
    {
        Shadow shadow = sShadows[light.shadowIndex];

        /* --- Get current normalized depth with bias --- */

        float bias = max(shadow.bias, shadow.slopeBias * (1.0 - NdotL));
        float currentDepth = toLightDist01 - bias;

        /* --- Get sampling direction and build orthonormal basis for perturbation --- */

        vec3 iL = -L;
        mat3 OBN = M_OrthonormalBasis(iL);

        /* --- Vogel disk PCF sampling --- */

        float softRadius = shadow.softness / float(textureSize(uTexShadowOmni, 0).x);

        float shadowAtten = 0.0;
        for (int i = 0; i < SHADOW_SAMPLES; ++i) {
            vec3 sampleDir = normalize(iL + OBN * vec3(params.diskRotation * VOGEL_DISK[i] * softRadius, 0.0));
            shadowAtten += step(currentDepth, texture(uTexShadowOmni, vec4(sampleDir, float(shadow.mapIndex))).r);
        }

        attenuation *= mix(1.0, shadowAtten / float(SHADOW_SAMPLES), shadow.opacity);
    }

    /* --- Return final contribution --- */

    return Lo * attenuation;
}

/* === IBL Functions === */

float IBL_GetSpecularOcclusion(float NdotV, float ao, float roughness)
{
    // Lagarde method: https://seblagarde.wordpress.com/wp-content/uploads/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
    return clamp(pow(NdotV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

vec3 IBL_SampleIrradiance(samplerCubeArray irradiance, uint index, vec3 N)
{
    return texture(irradiance, vec4(N, float(index))).rgb;
}

vec3 IBL_SampleIrradiance(samplerCubeArray irradiance, uint index, vec3 N, vec4 rotation)
{
    return texture(irradiance, vec4(M_Rotate3D(N, rotation), float(index))).rgb;
}

vec3 IBL_SamplePrefilter(samplerCubeArray prefilter, uint index, vec3 V, vec3 N, float roughness)
{
    float mipLevel = roughness * float(textureQueryLevels(prefilter) - 1);
    return textureLod(prefilter, vec4(reflect(-V, N), float(index)), mipLevel).rgb;
}

vec3 IBL_SamplePrefilter(samplerCubeArray prefilter, uint index, vec3 V, vec3 N, vec4 rotation, float roughness)
{
    float mipLevel = roughness * float(textureQueryLevels(prefilter) - 1);
    return textureLod(prefilter, vec4(M_Rotate3D(reflect(-V, N), rotation), float(index)), mipLevel).rgb;
}

vec3 IBL_MultiScattering(vec3 irradiance, vec3 radiance, vec3 diffuse, vec3 F0, vec2 brdf, float NdotV, float roughness)
{
    // Roughness dependent fresnel, from Fdez-Aguera
    vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
    vec3 kS = F0 + Fr * pow(1.0 - NdotV, 5.0);
    vec3 FssEss = kS * brdf.x + brdf.y;

    // Multiple scattering, from Fdez-Aguera
    float Ems = (1.0 - (brdf.x + brdf.y));
    vec3 Favg = F0 + (1.0 - F0) / 21.0;
    vec3 FmsEms = Ems * FssEss * Favg / (1.0 - Favg * Ems);
    vec3 kD = diffuse * (1.0 - FssEss - FmsEms);

    return irradiance * (FmsEms + kD) + radiance * FssEss;
}

/* === Program === */

void main()
{
    /* --- Call fragment override --- */

    FragmentOverride();

    /* --- Alpha cutoff with GLES (no pre-pass) --- */

    // TODO: Test pre-pass on mobile

//#if defined(GL_ES)
//    if (ALBEDO.a < sDrawUnique[uDrawUniqueIndex].alphaCutOff) {
//        discard;
//    }
//#endif

    /* --- Get occlusion from SSAO buffer --- */

#if defined(PREPASS)
    if (uEnv.ssaoEnabled) {
        float ssao = texture(uTexSsaoBuffer, gl_FragCoord.xy * uInvResolution).r;
        if (uEnv.ssaoPower != 1.0) ssao = pow(ssao, uEnv.ssaoPower);
        OCCLUSION *= mix(1.0, ssao, uEnv.ssaoIntensity);
    }
#endif

    /* --- Compute base material properties --- */

    const float SPECULAR = 0.5;
    float dielectric = 0.16 * SPECULAR * SPECULAR;
    vec3 F0 = mix(vec3(dielectric), ALBEDO.rgb, vec3(METALNESS));
    vec3 diffuse = ALBEDO.rgb * (1.0 - dielectric) * (1.0 - METALNESS);

    /* --- Calculation of the distance from the fragment to the camera in scene units --- */

    float zLinear = F_LinearizeDepth(gl_FragCoord.z, uFrustum.near, uFrustum.far);

    /* --- Sample normal and compute view direction vector --- */

#if defined(PREPASS)
    vec2 No = texelFetch(uTexNormalBuffer, ivec2(gl_FragCoord.xy), 0).rg;
    vec3 N = normalize(M_DecodeOctahedral(No));
#else
    vec3 N = M_NormalScale(NORMAL_MAP.rgb * 2.0 - 1.0, NORMAL_SCALE);
    N = normalize(vInt.tbn * N) * (gl_FrontFacing ? 1.0 : -1.0);
#endif

    vec3 V = normalize(uFrustum.position - vInt.position);

    /* --- Compute the dot product of the normal and view direction --- */

    float NdotV = dot(N, V);
    float cNdotV = max(NdotV, 1e-4); // Clamped to avoid division by zero

    /* --- Accumulation of direct lighting --- */

    vec3 Lo = vec3(0.0);

    if (uFrame.hasActiveLights)
    {
        /* --- Pre calculate some values --- */

        float alphaGGX = max(ROUGHNESS * ROUGHNESS, 1e-6);
        float aoLight = mix(1.0, OCCLUSION, AO_LIGHT_AFFECT);

        /* --- Compute the shadow sampling disk rotation --- */

        float r = M_TAU * M_HashIGN(gl_FragCoord.xy);
        float sr = sin(r), cr = cos(r);

        mat2 diskRotation = mat2(vec2(cr, -sr), vec2(sr, cr));

        /* --- Create commong light parameters struct --- */

        LightParams lightParams = LightParams(
            N, V, F0, diffuse * aoLight,
            cNdotV, alphaGGX,
            diskRotation
        );

        /* --- Getting the cluster index and the number of lights in the cluster --- */

        uvec3 clusterCoord = L_ClusterFromScreen(gl_FragCoord.xy / vec2(uFrame.screenSize),
            -zLinear, uFrame.clusterCount, uFrame.clusterSliceScale, uFrame.clusterSliceBias);

        uint clusterIndex = L_ClusterIndex(clusterCoord, uFrame.clusterCount);
        uint baseIndex = clusterIndex * uFrame.maxLightsPerCluster;
        uvec4 lightCounts = sClusters[clusterIndex];

        /* --- Loop through all light sources accumulating diffuse and specular light --- */

        uint offset = 0u; // dir
        for (uint i = 0u; i < lightCounts.x; i++) {
            uint lightIndex = sIndices[baseIndex + offset + i];
            Lo += LightDir(lightIndex, lightParams);
        }

        offset += lightCounts.x; // spot
        for (uint i = 0u; i < lightCounts.y; i++) {
            uint lightIndex = sIndices[baseIndex + offset + i];
            Lo += LightSpot(lightIndex, lightParams);
        }

        offset += lightCounts.y; // omni
        for (uint i = 0u; i < lightCounts.z; i++) {
            uint lightIndex = sIndices[baseIndex + offset + i];
            Lo += LightOmni(lightIndex, lightParams);
        }
    }

    /* --- Compute IBL contribution  --- */

    vec3 irradiance = vec3(0.0);
    vec3 radiance = vec3(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < uFrame.reflectionProbeCount; ++i) {
        ReflectionProbe probe = sReflectionProbes[i];
        float dist = length(vInt.position - probe.position);
        float weight = pow(clamp(1.0 - dist / probe.range, 0.0, 1.0), probe.falloff);
        if (weight > 0.0) {
            vec3 probeIrradiance = IBL_SampleIrradiance(uTexIrradiance, probe.index, N);
            vec3 probeRadiance = IBL_SamplePrefilter(uTexPrefilter, probe.index, V, N, ROUGHNESS);
            irradiance += probeIrradiance * weight;
            radiance += probeRadiance * weight;
            totalWeight += weight;
        }
    }

    if (totalWeight < 1.0 && uEnv.skyLightMapIndex >= 0) {
        vec3 skyIrradiance = IBL_SampleIrradiance(uTexIrradiance, uint(uEnv.skyLightMapIndex), N, uEnv.skyRotation);
        vec3 skyRadiance = IBL_SamplePrefilter(uTexPrefilter, uint(uEnv.skyLightMapIndex), V, N, uEnv.skyRotation, ROUGHNESS);
        skyRadiance = mix(skyRadiance, uEnv.fogColor, uEnv.fogSkyAffect); // Applies fog (by skyAffect) to the radiance
        float skyWeight = (1.0 - totalWeight) * uEnv.skyIntensity;
        irradiance += skyIrradiance * skyWeight;
        radiance += skyRadiance * skyWeight;
        totalWeight = 1.0;
    }
    else if (totalWeight > 1.0) {
        float invTotalWeight = 1.0 * totalWeight;
        irradiance *= invTotalWeight;
        radiance *= invTotalWeight;
        totalWeight = 1.0;
    }

    if (totalWeight > 0.0) {
        irradiance *= OCCLUSION;
        radiance *= IBL_GetSpecularOcclusion(NdotV, OCCLUSION, ROUGHNESS);
        vec2 brdf = texture(uTexBrdfLut, vec2(NdotV, ROUGHNESS)).xy;
        Lo += IBL_MultiScattering(irradiance, radiance, diffuse, F0, brdf, NdotV, ROUGHNESS);
    }
    else {
        Lo += diffuse * uEnv.ambientColor * OCCLUSION;
    }

    /* --- Compute the fog factor --- */

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

    /* --- Output the final lighting contribution with emission and fog --- */

    FragColor = vec4(mix(uEnv.fogColor, Lo + EMISSION, fogFactor), ALBEDO.a);

    /* DEBUG: Clusters */

    //vec4 dbgClusters = vec4(vec3((lightCounts.x + lightCounts.y + lightCounts.z) > 0 ? 1.0 : 0.0), 1.0);
    //vec4 dbgClusters = vec4(vec3(lightCounts), 1.0);
    //FragColor = mix(FragColor, dbgClusters, 0.05);
}
