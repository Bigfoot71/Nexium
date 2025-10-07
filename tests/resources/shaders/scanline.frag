// Vertex shader for 'custom_pass.c'

layout(std140) uniform StaticBuffer {
    float scanline_density;
    float scanline_intensity;
    float flicker_speed;
    float vignette_strength;
    float vignette_softness;
};

void fragment()
{
    /* --- Scanline --- */

    float scan = sin(TEXCOORD.y * scanline_density + TIME * flicker_speed);
    scan = scan * 0.5 + 0.5;
    float brightness = 1.0 - scanline_intensity * (1.0 - scan);

    /* --- Vignette --- */

    vec2 uv = TEXCOORD.xy - 0.5;
    float dist = length(uv * vignette_softness);
    float vignette = smoothstep(1.0, 0.0, dist * vignette_strength);

    /* --- Output --- */

    COLOR.rgb *= brightness * vignette;
}
