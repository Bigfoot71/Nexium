/* ssao_post.frag -- Fragment shader for applying SSAO to the scene
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

/* === Profile Specific === */

#ifdef GL_ES
precision mediump float;
#endif

/* === Varyings === */

layout(location = 0) in vec2 vTexCoord;

/* === Samplers === */

layout(binding = 0) uniform sampler2D uTexScene;
layout(binding = 1) uniform sampler2D uTexAO;

/* === Uniforms === */

layout(location = 0) uniform float uIntensity;
layout(location = 1) uniform float uPower;

/* === Fragments === */

layout(location = 0) out vec4 FragColor;

/* === Program === */

void main()
{
    vec3 scene = texture(uTexScene, vTexCoord).rgb;
    float ao = texture(uTexAO, vTexCoord).r;

    if (uPower != 1.0) {
        ao = pow(ao, uPower);
    }

    ao = mix(1.0, ao, uIntensity);

    FragColor = vec4(scene.rgb * ao, 1.0);
}
