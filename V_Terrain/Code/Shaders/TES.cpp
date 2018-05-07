#version 430 core

layout(quads, ccw, fractional_even_spacing) in;

in vec3 position[];
in vec3 normal[];
in vec2 UVs[];

out vec3 pos;
out vec3 norm;
out vec2 UV;

uniform  mat4 view_matrix;
uniform  mat4 projection_matrix;
uniform  mat4 model_matrix;

uniform  float fog_distance;
uniform  float water_height;

uniform  sampler2D heightmap;

void main(void)
{
    UV = mix(mix(UVs[0], UVs[1], gl_TessCoord.x), mix(UVs[3], UVs[2], gl_TessCoord.x), gl_TessCoord.y);
    vec3 position = mix(mix(position[0], position[1], gl_TessCoord.x), mix(position[3], position[2], gl_TessCoord.x), gl_TessCoord.y).xyz;
    norm = mix(mix(normal[0], normal[1], gl_TessCoord.x), mix(normal[3], normal[2], gl_TessCoord.x), gl_TessCoord.y);

    pos = position;
    gl_Position = projection_matrix * view_matrix * vec4(position, 1);
}
