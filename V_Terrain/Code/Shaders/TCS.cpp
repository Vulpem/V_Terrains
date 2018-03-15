#version 430 core

layout(vertices = 4) out;

in lowp float dist[];
in lowp vec2 UV[];

out lowp vec2 UVs[];

void main(void) {
    if (gl_InvocationID == 0)
    {
        float inner = max(10 - (dist[0] + dist[1] + dist[2] + dist[3])/1000.f, 1.f);
		float outter = inner;
        gl_TessLevelInner[0] = inner;
		gl_TessLevelInner[1] = inner;

        gl_TessLevelOuter[0] = outter;
        gl_TessLevelOuter[1] = outter;
        gl_TessLevelOuter[2] = outter;
		gl_TessLevelOuter[3] = outter;
    }
	UVs[0] = UV[0];
	UVs[1] = UV[1];
	UVs[2] = UV[2];
	UVs[3] = UV[3];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}