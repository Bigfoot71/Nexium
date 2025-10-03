// Material fragment shader test

in flat uint vEffectIndex;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float sinPattern(vec2 p, float frequency)
{
    return 0.5 + 0.5 * sin((p.x + p.y) * frequency);
}

float checker(vec2 p, float size)
{
    vec2 q = floor(p / size);
    return mod(q.x + q.y, 2.0);
}

float radial(vec2 p)
{
    vec2 c = vec2(0.5);
    float d = distance(p, c);
    return 1.0 - smoothstep(0.0, 0.5, d);
}

float effect(vec2 p)
{
    switch (vEffectIndex) {
    case 0: {
        const float pixelSize = 4.0;
        vec2 pp = floor(p * 100.0 / pixelSize) * pixelSize;
        return hash(pp + 0.0001 * TIME);
    }
    case 1: {
        return hash(p + 0.0001 * TIME);
    }
    case 2: {
        return sinPattern(p, 20.0);
    }
    case 3: {
        return checker(p * 100.0, 8.0);
    }
    case 4: {
        return radial(p);
    }
    default:
        break;
    }

    return 1.0;
}

void fragment()
{
    EMISSION *= ALBEDO.rgb * effect(vTexCoord);
}
