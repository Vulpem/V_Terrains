#version 430 core

layout(vertices = 4) out;

uniform  mat4 view_matrix;
uniform  mat4 projection_matrix;
uniform  mat4 model_matrix;

in  vec2 UV[];
in vec3 pos[];

out  vec2 UVs[];

uniform int tesselationDensity[];

//https://developer.nvidia.com/gameworks-vulkan-and-opengl-samples

float AdaptiveTessellation(vec3 p0, vec3 p1)
{
    vec3 center = (p0 + p1)*0.5f;
    vec4 clip2 = view_matrix * vec4(center, 1.0);
    vec4 clip4 = view_matrix * vec4(0, 0, 0, 1.0);
    float cameraDist = length(clip2.xyz);
    float cameraDistFromCenter = length(clip4.xyz);

    vec4 clip0 = projection_matrix * view_matrix * model_matrix * vec4(p0, 1.0);
    vec4 clip1 = projection_matrix * view_matrix * model_matrix * vec4(p1, 1.0);
    const float d = distance(clip0, clip1);
    float terrTessTriSize = 100.0f;
    return (1 + clamp((cameraDistFromCenter * 2 / cameraDist)*(d / terrTessTriSize), 1, 50));

}

void main(void) {
	
    gl_TessLevelOuter[0] = AdaptiveTessellation(pos[0], pos[3]);
    gl_TessLevelOuter[1] = AdaptiveTessellation(pos[0], pos[1]);
    gl_TessLevelOuter[2] = AdaptiveTessellation(pos[1], pos[2]);
    gl_TessLevelOuter[3] = AdaptiveTessellation(pos[3], pos[2]);
    gl_TessLevelInner[0] = gl_TessLevelInner[1] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2] + gl_TessLevelOuter[3])*0.25f;

	UVs[gl_InvocationID] = UV[gl_InvocationID];
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}