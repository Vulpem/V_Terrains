#version 430 core

layout(vertices = 3) out;

in lowp float distSqr[];

void main(void) {
    if (gl_InvocationID == 0)
    {
        float a = max(5 - (sqrt(distSqr[0]) + sqrt(distSqr[1]) + sqrt(distSqr[2]))/500.f, 1.f);
        gl_TessLevelInner[0] = a;
        gl_TessLevelOuter[0] = a;
        gl_TessLevelOuter[1] = a;
        gl_TessLevelOuter[2] = a;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}