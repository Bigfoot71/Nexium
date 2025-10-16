/* color.glsl -- Contains everything you need to manage colors
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

// Expects 0-1 range input.
vec3 C_LinearToSRGB(vec3 color)
{
    // Approximation from http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    return max(vec3(1.055) * pow(color, vec3(0.416666667)) - vec3(0.055), vec3(0.0));
}

// Expects 0-1 range input.
vec3 C_LinearFromSRGB(vec3 color)
{
    // Approximation from http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    return color * (color * (color * 0.305306011 + 0.682171111) + 0.012522878);
}

float C_LumaFromSRGB(vec3 color)
{
    // See: https://en.wikipedia.org/wiki/Rec._601
    return dot(color, vec3(0.299, 0.587, 0.114));
}

float C_LuminanceFromLinear(vec3 color)
{
    // See: https://en.wikipedia.org/wiki/Rec._709
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}
