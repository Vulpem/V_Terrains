#version 410 core

in  vec3 position;
in  vec2 texCoord;

uniform  mat4 model_matrix;
uniform  sampler2D heightmap;
uniform  float water_height;


out vec3 pos;
out vec2 UV;

void main()
{
    pos = position;
	UV = texCoord;
}







