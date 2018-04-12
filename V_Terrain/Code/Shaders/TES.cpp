#version 430 core

layout(quads, ccw) in;
in lowp vec2 UVs[];
in lowp float distance[];

out lowp vec2 UV;
out lowp float dist;

uniform lowp mat4 view_matrix;
uniform lowp mat4 projection_matrix;

uniform lowp vec3 position_offset;
uniform lowp float max_height;

uniform lowp float fog_distance;
uniform lowp float water_height;

uniform lowp sampler2D heightmap;

void main(void)
{
    dist = mix(mix(distance[0], distance[1], gl_TessCoord.x), mix(distance[3], distance[2], gl_TessCoord.x), gl_TessCoord.y);
    UV = mix(mix(UVs[0], UVs[1], gl_TessCoord.x), mix(UVs[3], UVs[2], gl_TessCoord.x), gl_TessCoord.y);
    vec3 position = mix(mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x), mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x), gl_TessCoord.y).xyz;

    lowp mat4 transform = projection_matrix * view_matrix;
    lowp vec4 heightMapVal = texture(heightmap, UV);

    vec3 finalPos = position + position_offset + vec3(0, heightMapVal.w * max_height, 0);
    finalPos.y = max(finalPos.y, water_height);
    gl_Position = transform * vec4(finalPos, 1);
}
