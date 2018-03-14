#version 430 core

layout(vertices = 3) out;

void main(void) {
    if (gl_InvocationID == 0)
    {
        float a = 1.f;
        float b = a;
        gl_TessLevelInner[0] = a;
        gl_TessLevelOuter[0] = a;
        gl_TessLevelOuter[1] = b;
        gl_TessLevelOuter[2] = b;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}