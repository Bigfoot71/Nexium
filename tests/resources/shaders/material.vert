// Material vertex shader for 'material_shader.c'

layout(std140) uniform StaticBuffer {
    float u_scale;
};

void vertex()
{
    POSITION *= u_scale;
}
