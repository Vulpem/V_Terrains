#version 430 core

layout(vertices = 4) out;

in lowp float dist[];
in lowp vec2 UV[];

out lowp vec2 UVs[];
patch out lowp float density;
patch out lowp float maxDensity;

void main(void) {
    if (gl_InvocationID == 0)
    {
        maxDensity = 40;

        const float inner = max(maxDensity - (dist[0] + dist[1] + dist[2] + dist[3])/1000.f, 1.f);
        const float outter = inner;
        gl_TessLevelInner[0] = inner;
		gl_TessLevelInner[1] = inner;

        gl_TessLevelOuter[0] = outter;
        gl_TessLevelOuter[1] = outter;
        gl_TessLevelOuter[2] = outter;
		gl_TessLevelOuter[3] = outter;

        density = inner;
    }
	UVs[gl_InvocationID] = UV[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}