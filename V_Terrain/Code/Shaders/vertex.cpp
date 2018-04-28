#version 330 core

in  vec3 position;
in  vec2 texCoord;

out  vec2 UV;
out  float dist;

uniform  mat4 view_matrix;
uniform  mat4 model_matrix;

uniform  vec3 observerPos;

uniform  sampler2D heightmap;

void main()
{
    gl_Position = vec4(position, 1);
    UV = texCoord;
    dist = length(position + vec4(model_matrix[3]).xyz + (vec4(view_matrix[3]) * view_matrix).xyz);
}