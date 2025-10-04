// Material fragment shader for 'material_shader.c'

layout(std140) uniform DynamicBuffer {
    vec4 u_color;
};

void fragment()
{
    ALBEDO *= u_color;
}
