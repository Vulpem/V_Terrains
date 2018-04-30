#version 330 core

in  vec3 position;
in  vec2 texCoord;

out  vec2 UV;

uniform  mat4 view_matrix;
uniform  mat4 model_matrix;

uniform  vec3 observerPos;

uniform  sampler2D heightmap;

void main()
{
    gl_Position = vec4(position, 1);
    UV = texCoord;
}