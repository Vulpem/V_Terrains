#version 330 core

in  vec3 position;
in  vec2 texCoord;

uniform  mat4 model_matrix;
uniform  sampler2D heightmap;
uniform  float water_height;


out vec3 pos;
out vec3 norm;
out vec2 UV;

void main()
{
    vec4 hmVal = texture2D(heightmap, texCoord);
    pos = (model_matrix * vec4(position + vec3(0, hmVal.w, 0), 1.f)).xyz;

    float realWaterHeight = water_height * model_matrix[1][1];
    if (pos.y < realWaterHeight)
    {
        pos.y = realWaterHeight;
        norm = vec3(0.f, 1.f, 0.f);
    }
    else
    {
        norm = hmVal.xyz * 2.f - 1.f;
    }
	UV = texCoord;
}
