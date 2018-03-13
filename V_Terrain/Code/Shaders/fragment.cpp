#version 330 core

in lowp vec3 pos;
in lowp float fog;
in lowp vec2 UV;

uniform lowp vec3 global_light_direction;

uniform lowp float ambient_min;
uniform lowp float max_height;
uniform lowp float water_height;
uniform lowp vec3 water_color;
uniform lowp vec3 fog_color;
uniform int render_chunk_borders;
uniform int render_heightmap;
uniform int render_light;

uniform lowp sampler2D heightmap;

out vec4 color;

void main()
{

    lowp vec4 heightmapVal = texture(heightmap, UV);
    lowp vec3 norm = vec3(heightmapVal.x * 2.f - 1.f, heightmapVal.y * 2.f - 1.f, heightmapVal.z * 2.f - 1.f);
    lowp float height = heightmapVal.w * max_height;

    lowp vec3 col;
    //Water
    if (render_heightmap == 0)
    {
        if (height <= water_height + water_height * 0.0001f)
        {
            col = water_color;
        }
        //Steep 2
        else if (norm.y < 0.1f)
        {
            if (height < (max_height - water_height) * 0.62f + water_height)
            {
                col = vec3(0.535f, 0.613f, 0.672f);
            }
            else
            {
                col = vec3(0.335f, 0.413f, 0.472f);
            }
        }
        //Steep
        else if (norm.y < 0.15f && height >(max_height - water_height) * 0.56f + water_height)
        {
            col = vec3(0.435f, 0.513f, 0.572f);
        }
        //Sand
        else if (height <= water_height + 30)
        {
            col = vec3(0.949f, 0.823f, 0.662f);
        }
        //Snow
        else if (height > (max_height - water_height) * 0.62f + water_height)
        {
            col = vec3(0.937f, 0.981f, 1.f);
        }
        //Default
        else
        {
            col = vec3(0.478f, 0.8f, 0.561f);
        }
    }
    else
    {
        col = vec3(heightmapVal.w, heightmapVal.w, heightmapVal.w);
    }

    if (render_chunk_borders != 0 && (UV.x <= 0.01f || UV.y <= 0.01f || UV.x >= 0.99 || UV.y >= 0.99))
    {
        col = vec3(1.f, 1.f, 1.f);
    }

	if (render_light != 0)
	{
		col *= max(dot(global_light_direction, norm), ambient_min);
	}
    color = vec4(mix(col, fog_color, fog), 1.f);
}