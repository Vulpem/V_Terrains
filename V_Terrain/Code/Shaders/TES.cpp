#version 430 core

layout(quads, ccw) in;
in lowp vec2 UVs[];

out lowp vec2 UV;

uniform lowp mat4 view_matrix;
uniform lowp mat4 projection_matrix;

uniform lowp vec3 position_offset;
uniform lowp float max_height;

uniform lowp float fog_distance;
uniform lowp float water_height;

uniform lowp sampler2D heightmap;

void main(void)
{
	UV = mix( mix(UVs[0], UVs[1], gl_TessCoord.x), mix(UVs[2], UVs[3], gl_TessCoord.x), gl_TessCoord.y );

	lowp mat4 transform = projection_matrix * view_matrix;
	lowp vec4 heightMapVal = texture(heightmap, UV);

	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec3 position = mix(p1, p2, gl_TessCoord.y).xyz;
	vec3 pos = position + position_offset /*+ vec3(0, heightMapVal.w * max_height, 0)*/;
	pos.y = max(pos.y, water_height);
	gl_Position = transform * vec4(pos, 1);
    //gl_Position = (gl_TessCoord.x*gl_in[0].gl_Position + gl_TessCoord.y*gl_in[1].gl_Position + gl_TessCoord.z*gl_in[2].gl_Position);
}