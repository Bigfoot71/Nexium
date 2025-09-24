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
