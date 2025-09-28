/* lights.glsl -- Contains everything you need to manage lights
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Constants === */

#define LIGHT_DIR    0
#define LIGHT_SPOT   1
#define LIGHT_OMNI   2

/* === Structures === */

struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    float energy;
    float specular;
    float range;
    float attenuation;
    float innerCutOff;
    float outerCutOff;
    uint layerMask;
    uint cullMask;
    int shadowIndex;
    int type;
};

struct Shadow {
    mat4 viewProj;
    float bleedingBias;
    float softness;
    float lambda;
    int mapIndex;
};

struct Cluster {
    vec3 minBounds;
    vec3 maxBounds;
};

/* === Functions === */

uvec3 L_ClusterFromScreen(vec2 screenUV, float viewZ, uvec3 clusterCount, float sliceScale, float sliceBias)
{
    uvec2 tileCoord = uvec2(screenUV * vec2(clusterCount.xy));
    tileCoord = min(tileCoord, clusterCount.xy - 1u);

    uint zSlice = uint(max(log2(-viewZ) * sliceScale + sliceBias, 0.0));
    zSlice = min(zSlice, clusterCount.z - 1u);

    return uvec3(tileCoord, zSlice);
}

uint L_ClusterIndex(uvec3 clusterCoord, uvec3 clusterCount)
{
    return clusterCoord.z * clusterCount.x * clusterCount.y +
           clusterCoord.y * clusterCount.x +
           clusterCoord.x;
}
