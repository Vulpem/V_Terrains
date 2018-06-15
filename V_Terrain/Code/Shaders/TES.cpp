#version 410 core

layout(quads, ccw) in;

in vec3 position[];
in vec2 UVs[];

out vec3 pos;
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

    vec4 hmVal = texture2D(heightmap, UV);
    pos = (model_matrix * vec4(position + vec3(0, hmVal.w, 0), 1.f)).xyz;

    float realWaterHeight = water_height * model_matrix[1][1];
    if (pos.y < realWaterHeight)
    {
        pos.y = realWaterHeight;
    }

    gl_Position = projection_matrix * view_matrix * vec4(pos, 1);
}





