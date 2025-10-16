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
#endif

/* === Includes === */

#include "../include/environment.glsl"
#include "../include/frustum.glsl"
#include "../include/lights.glsl"
#include "../include/frame.glsl"
#include "../include/draw.glsl"
#include "../include/math.glsl"
#include "../include/pbr.glsl"

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

float ReduceLightBleeding(float pMax, float amount)
{
    return clamp((pMax - amount) / (1.0 - amount), 0.0, 1.0);
}

float ShadowDir(in Light light)
{
    Shadow shadow = sShadows[light.shadowIndex];

    vec4 projPos = shadow.viewProj * vec4(vInt.position, 1.0);
    vec3 projCoords = projPos.xyz / projPos.w * 0.5 + 0.5;

    float d01 = length(vInt.position - light.position) / light.range;
    vec2 m = texture(uTexShadowDir, vec3(projCoords.xy, float(shadow.mapIndex))).rg;

#ifdef GL_ES
    // VSM
    float dx = d01 - m.x;
    float v  = max(m.y - m.x * m.x, 1e-4);
    float p  = v / (v + dx * dx);
    float factor = ReduceLightBleeding(p, shadow.bleedingBias);
#else
    // EVSM
    float nExp = exp(-shadow.lambda * d01);
    float pExp = exp(+shadow.lambda * d01);
    float p = min(m.x * nExp, m.y * pExp);
    float factor = ReduceLightBleeding(p, shadow.bleedingBias);
#endif

    vec3 distToBorder = min(projCoords, 1.0 - projCoords);
    float minDist = min(distToBorder.x, min(distToBorder.y, distToBorder.z));

    const float fadeStart = 0.85; // Start of attenuation (85% of the map)
    float edgeFade = smoothstep(0.0, 1.0 - fadeStart, minDist);

    return mix(1.0, factor, edgeFade);
}

float ShadowSpot(in Light light)
{
    Shadow shadow = sShadows[light.shadowIndex];

    vec4 projPos = shadow.viewProj * vec4(vInt.position, 1.0);
    vec3 projCoords = projPos.xyz / projPos.w * 0.5 + 0.5;

    float d01 = length(vInt.position - light.position) / light.range;
    vec2 m = texture(uTexShadowSpot, vec3(projCoords.xy, float(shadow.mapIndex))).rg;

#ifdef GL_ES
    // VSM
    float dx = d01 - m.x;
    float v  = max(m.y - m.x * m.x, 1e-4);
    float p  = v / (v + dx * dx);
    float factor = ReduceLightBleeding(p, shadow.bleedingBias);
#else
    // EVSM
    float nExp = exp(-shadow.lambda * d01);
    float pExp = exp(+shadow.lambda * d01);
    float p = min(m.x * nExp, m.y * pExp);
    float factor = ReduceLightBleeding(p, shadow.bleedingBias);
#endif

    vec2 distToBorder = min(projCoords.xy, 1.0 - projCoords.xy);
    float minDist = min(distToBorder.x, distToBorder.y);

    const float fadeStart = 0.85; // Start of attenuation (85% of the map)
    float edgeFade = smoothstep(0.0, 1.0 - fadeStart, minDist);

    return mix(1.0, factor, edgeFade);
}

float ShadowOmni(in Light light)
{
    Shadow shadow = sShadows[light.shadowIndex];

    vec3 lightToFrag = vInt.position - light.position;
    float dist = length(lightToFrag);

    vec3 direction = lightToFrag / dist;
    float d01 = dist / light.range;

    vec2 m = texture(uTexShadowOmni, vec4(direction, float(shadow.mapIndex))).rg;

#ifdef GL_ES
    // VSM
    float dx = d01 - m.x;
    float v  = max(m.y - m.x * m.x, 1e-4);
    float p  = v / (v + dx * dx);
    float factor = ReduceLightBleeding(p, shadow.bleedingBias);
#else
    // EVSM
    float nExp = exp(-shadow.lambda * d01);
    float pExp = exp(+shadow.lambda * d01);
    float p = min(m.x * nExp, m.y * pExp);
    float factor = ReduceLightBleeding(p, shadow.bleedingBias);
#endif

    return factor;
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
                shadow = ShadowDir(light);
                break;
            case LIGHT_SPOT:
                shadow = ShadowSpot(light);
                break;
            case LIGHT_OMNI:
                shadow = ShadowOmni(light);
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
