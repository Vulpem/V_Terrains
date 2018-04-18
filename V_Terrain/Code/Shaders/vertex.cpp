#version 330 core

in lowp vec3 position;
in lowp vec2 texCoord;

out lowp vec2 UV;
out lowp float dist;

uniform lowp mat4 view_matrix;
uniform lowp mat4 model_matrix;

uniform lowp vec3 observerPos;

uniform lowp sampler2D heightmap;

void main()
{
    gl_Position = vec4(position, 1);
    UV = texCoord;
    dist = length(position + vec4(model_matrix[3]).xyz + (vec4(view_matrix[3]) * view_matrix).xyz);
}