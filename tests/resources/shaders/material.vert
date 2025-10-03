// Material vertex shader test

out flat uint vEffectIndex;

void vertex()
{
    float scale = 0.5 + sin(M_PI * TIME + INSTANCE_DATA.y) * 0.5;
    vEffectIndex = uint(INSTANCE_DATA.x) % 4;
    POSITION *= 0.5 + scale;
}
