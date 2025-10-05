// Material fragment shader for 'material_shader.c'

layout(std140) uniform DynamicBuffer {
    vec4 u_color;
};

uniform sampler2D Texture0;

void fragment()
{
    ALBEDO *= u_color;
    EMISSION = texture(Texture0, TEXCOORD).rgb;
    EMISSION *= ALBEDO.rgb;
}
