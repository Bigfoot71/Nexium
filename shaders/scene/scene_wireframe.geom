/* scene_wireframe.geom -- Geometry shader rendering mesh edges as wireframe lines.
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

/* === Vertex Inputs === */

layout(location = 0) in VaryInternal {
    vec3 position;
    vec2 texCoord;
    vec4 color;
    mat3 tbn;
} vInt[];

layout(location = 10) in VaryUser {
    smooth vec4 data4f;
    flat ivec4 data4i;
} vUsr[];

/* === Fragment Outputs === */

layout(location = 0) out VaryInternal {
    vec3 position;
    vec2 texCoord;
    vec4 color;
    mat3 tbn;
} gInt;

layout(location = 10) out VaryUser {
    smooth vec4 data4f;
    flat ivec4 data4i;
} gUsr;

/* === Geometry In/Out === */

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

/* === Program === */

void main()
{
    const int next[3] = int[3](1, 2, 0);

    for(int i = 0; i < 3; ++i)
    {
        gl_Position = gl_in[i].gl_Position;

        gInt.position = vInt[i].position;
        gInt.texCoord = vInt[i].texCoord;
        gInt.color = vInt[i].color;
        gInt.tbn = vInt[i].tbn;

        gUsr.data4f = vUsr[i].data4f;
        gUsr.data4i = vUsr[i].data4i;

        EmitVertex();

        int j = next[i];

        gl_Position = gl_in[j].gl_Position;

        gInt.position = vInt[j].position;
        gInt.texCoord = vInt[j].texCoord;
        gInt.color = vInt[j].color;
        gInt.tbn = vInt[j].tbn;

        gUsr.data4f = vUsr[j].data4f;
        gUsr.data4i = vUsr[j].data4i;

        EmitVertex();

        EndPrimitive();
    }
}
