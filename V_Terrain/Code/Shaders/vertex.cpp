#version 330 core

in lowp vec3 position;
in lowp vec2 texCoord;

out lowp vec2 UV;
out lowp float dist;

uniform lowp mat4 view_matrix;
uniform lowp mat4 projection_matrix;

uniform lowp vec3 position_offset;

uniform lowp sampler2D heightmap;

void main()
{
	gl_Position = vec4(position, 1);
    UV = texCoord;

	vec4 outPos = projection_matrix * view_matrix * vec4(position + position_offset, 1);
	dist = sqrt(outPos.x * outPos.x + outPos.y * outPos.y + outPos.z * outPos.z);
    //fog = min( distSqr / (fog_distance * fog_distance), 1);
}