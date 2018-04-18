#version 430 core

layout(vertices = 4) out;

in lowp float dist[];
in lowp vec2 UV[];

out lowp vec2 UVs[];
out lowp float distance[];

uniform int tesselationDensity[];

void main(void) {
	if (gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = tesselationDensity[0];
		gl_TessLevelInner[1] = tesselationDensity[1];

		gl_TessLevelOuter[0] = tesselationDensity[2];
		gl_TessLevelOuter[1] = tesselationDensity[3];
		gl_TessLevelOuter[2] = tesselationDensity[4];
		gl_TessLevelOuter[3] = tesselationDensity[5];
	}
	distance[gl_InvocationID] = dist[gl_InvocationID];
	UVs[gl_InvocationID] = UV[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}