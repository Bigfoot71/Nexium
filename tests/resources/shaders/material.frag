// Material fragment shader test

float fade(float t)
{
    return t * t * (3.0 - 2.0 * t);
}

vec2 fade(vec2 t)
{
    return vec2(fade(t.x), fade(t.y));
}

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = fade(f);

    return mix(
        mix(a, b, u.x),
        mix(c, d, u.x),
        u.y
    );
}

void fragment()
{
    EMISSION *= 0.1 * noise(100.0 * vTexCoord);
}
