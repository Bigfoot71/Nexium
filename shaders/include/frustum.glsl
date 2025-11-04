/* utils.glsl -- Contains small general utility functions
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Structures === */

struct INX_Frustum {
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
};

/* === Functions === */

float F_LinearizeDepth(float depth, float near, float far)
{
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float F_LinearizeDepth01(float depth, float near, float far)
{
    float z = F_LinearizeDepth(depth, near, far);
    return (z - near) / (far - near);
}
