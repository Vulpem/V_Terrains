#version 330 core

in lowp vec3 position;
in lowp vec2 texCoord;

out lowp vec3 pos;
out lowp float fog;
out lowp vec2 UV;

out lowp float dist;

uniform lowp mat4 view_matrix;
uniform lowp mat4 projection_matrix;

uniform lowp vec3 position_offset;
uniform lowp float max_height;

uniform lowp float fog_distance;
uniform lowp float water_height;

uniform lowp sampler2D heightmap;

void main()
{
    lowp mat4 transform = projection_matrix * view_matrix;
    lowp vec4 heightMapVal = texture(heightmap, texCoord);
    pos = position /*+ position_offset + vec3(0, heightMapVal.w * max_height, 0)*/;
    pos.y = max(pos.y, water_height);

	gl_Position = vec4(pos, 1); // outPos;

	vec4 outPos = transform * vec4(pos + position_offset, 1);
	dist = sqrt(outPos.x * outPos.x + outPos.y * outPos.y + outPos.z * outPos.z);
    //fog = min( distSqr / (fog_distance * fog_distance), 1);

    UV = texCoord;
}