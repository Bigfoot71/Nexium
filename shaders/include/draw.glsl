/* draw.glsl -- Contains everything you need to manage draw calls
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

struct ModelData {
    mat4 matModel;
    mat4 matNormal;
    int boneOffset;
    bool instancing;
    bool skinning;
};

struct MeshData {
    vec4 albedoColor;
    vec3 emissionColor;
    float emissionEnergy;
    float aoLightAffect;
    float occlusion;
    float roughness;
    float metalness;
    float normalScale;
    float alphaCutOff;
    float depthOffset;
    float depthScale;
    vec2 texOffset;
    vec2 texScale;
    int billboard;
    uint layerMask;
};
