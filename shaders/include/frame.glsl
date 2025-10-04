/* billboard.glsl -- Contains everything related to frame infos
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
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
