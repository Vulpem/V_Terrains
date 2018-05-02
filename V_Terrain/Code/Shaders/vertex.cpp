#version 330 core

in  vec3 position;
in  vec2 texCoord;

out  vec2 UV;
out vec3 pos;

uniform  mat4 view_matrix;
uniform  mat4 model_matrix;

uniform  vec3 observerPos;

uniform  sampler2D heightmap;

void main()
{
    pos = position;
    gl_Position = vec4(position, 1);
    UV = texCoord;
}