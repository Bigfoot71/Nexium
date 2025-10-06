// Material vertex shader for 'instanced_material_shader.c'

void vertex()
{
    float scale = 0.5 + sin(M_PI * TIME + INSTANCE_DATA.y) * 0.5;
    VARYINGI.x = int(INSTANCE_DATA.x) % 5;
    POSITION *= 0.5 + scale;
}
