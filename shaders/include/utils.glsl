/* utils.glsl -- Contains small general utility functions
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Functions === */

float U_LinearizeDepth(float depth, float near, float far)
{
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float U_LinearizeDepth01(float depth, float near, float far)
{
    float z = U_LinearizeDepth(depth, near, far);
    return (z - near) / (far - near);
}
