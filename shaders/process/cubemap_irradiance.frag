/* cubemap_irradiance.frag -- Irradiance cubemap generation fragment shader
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

/* === Inlcudes === */

#include "../include/math.glsl"

/* === Varyings === */

layout(location = 0) in vec3 vPosition;

/* === Uniforms === */

layout(binding = 0) uniform samplerCube uTexCubemap;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Program === */

void main()
{
    // The world vector acts as the normal of a tangent surface
    // from the origin, aligned to WorldPos. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.

    mat3 TBN = M_OrthonormalBasis(normalize(vPosition));

    vec3 irradiance = vec3(0.0);
    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    for (float phi = 0.0; phi < 2.0 * M_PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * M_PI; theta += sampleDelta)
        {
            // Spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

            // Tangent space to world
            vec3 sampleVec = TBN * tangentSample;

            irradiance += texture(uTexCubemap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = M_PI * irradiance * (1.0 / float(nrSamples));
    FragColor = vec4(irradiance, 1.0);
}
