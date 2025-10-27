/* billboard.glsl -- Contains everything related to frame infos
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#if defined(SHADOW)

/**
 * Used in 'shadow' shaders (render to shadow maps)
 */
struct Frame {
    mat4 lightViewProj;
    vec3 lightPosition;
    float lightRange;
    int lightType;
    float elapsedTime;
};

#else

/**
 * Used in 'forward' and 'prepass' shaders
 */
struct Frame {
    uvec2 screenSize;               // Render target dimensions
    uvec3 clusterCount;
    uint  maxLightsPerCluster;
    float clusterSliceScale;
    float clusterSliceBias;
    float elapsedTime;
    bool hasActiveLights;
    bool hasProbe;
};

#endif // SHADOW
