/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision highp float;
#endif

/* === Includes === */

#include "../include/lights.glsl"
#include "../include/utils.glsl"
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

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec4 vColor;
layout(location = 3) in mat3 vTBN;

/* === Storage Buffers === */

layout(std430, binding = 0) buffer LightBuffer {
    Light sLights[];
};

layout(std430, binding = 1) buffer ShadowBuffer {
    Shadow sShadows[];
};

layout(std430, binding = 2) buffer TileBuffer {
    uint sClusters[]; //< Contains number of lights for each tile
};

layout(std430, binding = 3) buffer IndexBuffer {
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

layout(binding = 7) uniform highp samplerCubeArray uTexShadowCube;
layout(binding = 8) uniform highp sampler2DArray uTexShadow2D;

/* === Uniform Buffers === */

layout(std140, binding = 0) uniform ViewFrustum {
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 invViewProj;
    mat4 invView;
    mat4 invProj;
    vec3 position;
    uint cullMask;
    float near;
    float far;
} uFrustum;

/* === Uniforms === */

layout(location = 7) uniform bool uHasActiveLights;

layout(location = 8) uniform uvec2 uScreenSize;             //< Must match the dimensions of the render target
layout(location = 9) uniform uvec3 uClusterCount;
layout(location = 10) uniform uint uMaxLightsPerCluster;
layout(location = 11) uniform float uClusterSliceScale;
layout(location = 12) uniform float uClusterSliceBias;

layout(location = 13) uniform vec3 uAmbientColor;

layout(location = 14) uniform bool uHasProbe;
layout(location = 15) uniform vec4 uQuatProbe;
layout(location = 16) uniform float uProbeDiffuseFactor;
layout(location = 17) uniform float uProbeSpecularFactor;
layout(location = 18) uniform int uProbePrefilterMipCount;

layout(location = 19) uniform vec3 uEmissionColor;
layout(location = 20) uniform float uEmissionEnergy;
layout(location = 21) uniform float uAoLightAffect;
layout(location = 22) uniform float uOcclusion;
layout(location = 23) uniform float uRoughness;
layout(location = 24) uniform float uMetalness;
layout(location = 25) uniform float uNormalScale;
layout(location = 26) uniform float uAlphaCutOff;
layout(location = 27) uniform uint uLayerMask;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Lighting Functions === */

float Diffuse(float cLdotH, float cNdotV, float cNdotL, float roughness)
{
    float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
    float FdV = 1.0 + FD90_minus_1 * PBR_SchlickFresnel(cNdotV);
    float FdL = 1.0 + FD90_minus_1 * PBR_SchlickFresnel(cNdotL);

    return (1.0 / M_PI) * (FdV * FdL * cNdotL); // Diffuse BRDF (Burley)
}

vec3 Specular(vec3 F0, float cLdotH, float cNdotH, float cNdotV, float cNdotL, float roughness)
{
    roughness = max(roughness, 1e-3);

    float alphaGGX = roughness * roughness;
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

float ReduceLightBleeding(float pMax, float amount)
{
    return clamp((pMax - amount) / (1.0 - amount), 0.0, 1.0);
}

float ShadowCube(in Light light, float cNdotL)
{
    Shadow shadow = sShadows[light.shadowIndex];

    /* --- Light vector and direction --- */

    vec3 lightToFrag = vPosition - light.position;
    vec3 direction = normalize(lightToFrag);

    /* --- Normalized distance --- */

    float d01 = length(lightToFrag) / light.range;

    /* --- Build orthonormal basis for perturbation --- */

    mat3 TBN = M_OrthonormalBasis(direction);

    /* --- Generate an additional debanding rotation for the poisson disk --- */

    float r = M_TAU * InterleavedGradientNoise(gl_FragCoord.xy);
    float sr = sin(r);
    float cr = cos(r);
    mat2 diskRot = mat2(vec2(cr, -sr), vec2(sr, cr));

    /* --- Approximation of the size of the penumbra --- */

    float penumbraSize = shadow.softness * (1.0 + d01 * 2.0);

    /* --- Poisson Disk blur --- */

    float factor = 0.0;
    for(int i = 0; i < SHADOW_SAMPLES; i++)
    {
        vec2 diskOffset = diskRot * POISSON_DISK[i] * penumbraSize;
        vec3 sampleDir = normalize(TBN * vec3(diskOffset.xy, 1.0));

        vec2 m = texture(uTexShadowCube, vec4(sampleDir, float(shadow.mapIndex))).rg;

    #ifdef GL_ES
        // VSM
        float v = max(m.y - m.x * m.x, 1e-4);
        float p = v / (v + (d01 - m.x) * (d01 - m.x));
        factor += ReduceLightBleeding(p, shadow.bleedingBias);
    #else
        // EVSM
        float pT = m.x * exp(-shadow.lambda * d01);
        float nT = m.y * exp(+shadow.lambda * d01);
        factor += ReduceLightBleeding(min(pT, nT), shadow.bleedingBias);
    #endif
    }

    return factor / float(SHADOW_SAMPLES);
}

float Shadow2D(in Light light, float cNdotL)
{
    Shadow shadow = sShadows[light.shadowIndex];

    /* --- Light space projection --- */

    vec4 projPos = shadow.viewProj * vec4(vPosition, 1.0);
    vec3 projCoords = projPos.xyz / projPos.w * 0.5 + 0.5;

    /* --- Shadow map bounds check --- */

    if (any(greaterThan(projCoords.xyz, vec3(1.0))) ||
        any(lessThan(projCoords.xyz, vec3(0.0)))) {
        return 1.0;
    }

    /* --- Normalized distance --- */

    float d01 = length(vPosition - light.position) / light.range;

    /* --- Generate an additional debanding rotation for the poisson disk --- */

    float r = M_TAU * InterleavedGradientNoise(gl_FragCoord.xy);
    float sr = sin(r);
    float cr = cos(r);
    mat2 diskRot = mat2(vec2(cr, -sr), vec2(sr, cr));

    /* --- Approximation of the size of the penumbra --- */

    float penumbraSize = shadow.softness * (1.0 + d01 * 2.0);

    /* --- Poisson Disk blur --- */

    float factor = 0.0;
    for(int i = 0; i < SHADOW_SAMPLES; i++)
    {
        vec2 offset = diskRot * POISSON_DISK[i] * penumbraSize;
        vec2 sampleUV = projCoords.xy + offset;

        vec2 m = texture(uTexShadow2D, vec3(sampleUV, float(shadow.mapIndex))).rg;

    #ifdef GL_ES
        // VSM
        float v = max(m.y - m.x * m.x, 1e-4);
        float p = v / (v + (d01 - m.x) * (d01 - m.x));
        factor += ReduceLightBleeding(p, shadow.bleedingBias);
    #else
        // EVSM
        float pT = m.x * exp(-shadow.lambda * d01);
        float nT = m.y * exp(+shadow.lambda * d01);
        factor += ReduceLightBleeding(min(pT, nT), shadow.bleedingBias);
    #endif
    }

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
    /* Sample albedo texture */

    vec4 albedo = vColor * texture(uTexAlbedo, vTexCoord);

    // TODO: Alpha scissor is unnecessary after a depth pre-pass
    if (albedo.a < uAlphaCutOff) discard;

    /* Sample emission texture */

    vec3 emission = uEmissionColor * texture(uTexEmission, vTexCoord).rgb;
    emission *= uEmissionEnergy;

    /* Sample ORM texture and extract values */

    vec3 orm = texture(uTexORM, vTexCoord).rgb;

    float occlusion = uOcclusion * orm.x;
    float roughness = uRoughness * orm.y;
    float metalness = uMetalness * orm.z;

    /* Compute F0 (reflectance at normal incidence) based on the metallic factor */

    vec3 F0 = PBR_ComputeF0(metalness, 0.5, albedo.rgb);

    /* Sample normal and compute view direction vector */

    vec3 N = normalize(vTBN * NormalScale(texture(uTexNormal, vTexCoord).rgb * 2.0 - 1.0, uNormalScale));
    vec3 V = normalize(uFrustum.position - vPosition);

    /* Compute the dot product of the normal and view direction */

    float NdotV = dot(N, V);
    float cNdotV = max(NdotV, 1e-4); // Clamped to avoid division by zero

    /* Getting the cluster index and the number of lights in the cluster */

    uvec3 clusterCoord = L_ClusterFromScreen(gl_FragCoord.xy / vec2(uScreenSize),
        -U_LinearizeDepth(gl_FragCoord.z, uFrustum.near, uFrustum.far),
        uClusterCount, uClusterSliceScale, uClusterSliceBias);

    uint clusterIndex = L_ClusterIndex(clusterCoord, uClusterCount);
    uint lightCount = sClusters[clusterIndex];

    // Branchless early exit, if there are no active lights,
    // sClusters may contain stale/garbage values otherwise.
    lightCount *= uint(uHasActiveLights);

    /* Loop through all light sources accumulating diffuse and specular light */

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (uint i = 0u; i < lightCount; i++)
    {
        /* Calculate light index and get light */

        uint lightIndex = sIndices[clusterIndex * uMaxLightsPerCluster + i];
        Light light = sLights[lightIndex];

        /* Compute light direction */

        vec3 L = (light.type != LIGHT_DIR)
            ? normalize(light.position - vPosition) : -light.direction;

        /* Compute the dot product of the normal and light direction */

        float NdotL = max(dot(N, L), 0.0);
        float cNdotL = min(NdotL, 1.0); // clamped NdotL

        /* Compute the halfway vector between the view and light directions */

        vec3 H = normalize(V + L);

        float LdotH = max(dot(L, H), 0.0);
        float cLdotH = min(dot(L, H), 1.0);

        float NdotH = max(dot(N, H), 0.0);
        float cNdotH = min(NdotH, 1.0);

        /* Compute light color energy */

        vec3 lightColE = light.color * light.energy;

        /* Compute diffuse lighting */

        float diffuseStrength = 1.0 - metalness;  // 0.0 for pure metal, 1.0 for dielectric
        vec3 diffLight = lightColE * Diffuse(cLdotH, cNdotV, cNdotL, roughness) * diffuseStrength;

        /* Compute specular lighting */

        vec3 specLight =  Specular(F0, cLdotH, cNdotH, cNdotV, cNdotL, roughness);
        specLight *= lightColE * light.specular;

        /* Apply shadow factor if the light casts shadows */

        float shadow = 1.0;
        if (light.shadowIndex >= 0) {
            if (light.type == LIGHT_OMNI) {
                shadow = ShadowCube(light, cNdotL);
            }
            else {
                shadow = Shadow2D(light, cNdotL);
            }
        }

        /* Apply attenuation based on the distance from the light */

        if (light.type != LIGHT_DIR) {
            float dist = length(light.position - vPosition);
            float atten = 1.0 - clamp(dist / light.range, 0.0, 1.0);
            shadow *= atten * light.attenuation;
        }

        /* Apply spotlight effect if the light is a spotlight */

        if (light.type == LIGHT_SPOT) {
            float theta = dot(L, -light.direction);
            float epsilon = (light.innerCutOff - light.outerCutOff);
            shadow *= smoothstep(0.0, 1.0, (theta - light.outerCutOff) / epsilon);
        }

        /* Accumulate the diffuse and specular lighting contributions */

        bool validLayer = ((light.cullMask & uLayerMask) != 0u);
        float contribution = shadow * float(validLayer);

        diffuse += diffLight * contribution;
        specular += specLight * contribution;
    }

    /* Compute AO light affect */

    float aoLightAffect = mix(1.0, occlusion, uAoLightAffect);
    specular *= aoLightAffect;
    diffuse *= aoLightAffect;

    /* Ambient diffuse from sky */

    vec3 skyDiffuse = uAmbientColor;

    if (uHasProbe) {
        vec3 Nr = M_Rotate3D(N, uQuatProbe);
        skyDiffuse = texture(uTexProbeIrradiance, Nr).rgb;
    }

    vec3 kS = IBL_FresnelSchlickRoughness(cNdotV, F0, roughness);
    vec3 kD = (1.0 - kS) * (1.0 - metalness);

    skyDiffuse *= kD * uProbeDiffuseFactor * occlusion;

    /* Ambient specular from sky */

    vec3 skySpecular = uAmbientColor;

    if (uHasProbe) {
        vec3 R = M_Rotate3D(reflect(-V, N), uQuatProbe);
        float mipLevel = roughness * (float(uProbePrefilterMipCount) - 1.0);
        skySpecular = textureLod(uTexProbePrefilter, R, mipLevel).rgb;
    }

    float specOcclusion = IBL_GetSpecularOcclusion(cNdotV, occlusion, roughness);
    vec3 specBRDF = IBL_GetMultiScatterBRDF(cNdotV, roughness, F0, metalness);
    skySpecular *= specBRDF * uProbeSpecularFactor * specOcclusion;

    /* Compute the final fragment color by combining albedo and lighting contributions */

    FragColor.rgb = albedo.rgb * (skyDiffuse + diffuse);
    FragColor.rgb += skySpecular + specular;
    FragColor.rgb += emission;
    FragColor.a = albedo.a;

    /* DEBUG: Tiles */

    //FragColor = (lightCount == 0u) ? vec4(0.0) : vec4(1.0);
}
