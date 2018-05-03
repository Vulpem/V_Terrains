#version 330 core

in  vec3 position;
in  vec2 texCoord;

uniform  mat4 model_matrix;
uniform  sampler2D heightmap;

out  vec2 UV;
out vec3 pos;

void main()
{
	pos = (model_matrix * vec4((position + vec3(0, texture(heightmap, UV).w, 0)), 1.f)).xyz;
	gl_Position = vec4(position, 1);
	UV = texCoord;
}
