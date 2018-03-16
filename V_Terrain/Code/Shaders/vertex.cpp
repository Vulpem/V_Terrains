#version 330 core

in lowp vec3 position;
in lowp vec2 texCoord;

out lowp vec2 UV;
out lowp float dist;

uniform lowp mat4 view_matrix;
uniform lowp mat4 projection_matrix;
uniform lowp vec3 observerPos;

uniform lowp vec3 position_offset;

uniform lowp sampler2D heightmap;

void main()
{
    gl_Position = vec4(position, 1);
    UV = texCoord;
    dist = length(position + position_offset + (vec4(view_matrix[3]) * view_matrix).xyz);
}