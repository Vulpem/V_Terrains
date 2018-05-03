#version 430 core

layout(quads, ccw, fractional_even_spacing) in;
in  vec2 UVs[];

out  vec2 UV;

uniform  mat4 view_matrix;
uniform  mat4 projection_matrix;
uniform  mat4 model_matrix;

uniform  float fog_distance;
uniform  float water_height;

uniform  sampler2D heightmap;

void main(void)
{
    UV = mix(mix(UVs[0], UVs[1], gl_TessCoord.x), mix(UVs[3], UVs[2], gl_TessCoord.x), gl_TessCoord.y);
    vec3 position = mix(mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x), mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x), gl_TessCoord.y).xyz;

     mat4 transform = projection_matrix * view_matrix * model_matrix;
     vec4 heightMapVal = texture(heightmap, UV);

    vec3 finalPos = position + vec3(0, heightMapVal.w, 0);
    finalPos.y = max(finalPos.y, water_height);
    gl_Position = transform * vec4(finalPos, 1);
}
