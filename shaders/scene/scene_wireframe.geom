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

layout(location = 0) in vec3 vPosition[];
layout(location = 1) in vec2 vTexCoord[];
layout(location = 2) in vec4 vColor[];
layout(location = 3) in mat3 vTBN[];

/* === Fragment Outputs === */

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec2 gTexCoord;
layout(location = 2) out vec4 gColor;
layout(location = 3) out mat3 gTBN;

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
        gPosition = vPosition[i];
        gTexCoord = vTexCoord[i];
        gColor = vColor[i];
        gTBN = vTBN[i];
        EmitVertex();

        int j = next[i];

        gl_Position = gl_in[j].gl_Position;
        gPosition = vPosition[j];
        gTexCoord = vTexCoord[j];
        gColor = vColor[j];
        gTBN = vTBN[j];
        EmitVertex();

        EndPrimitive();
    }
}
