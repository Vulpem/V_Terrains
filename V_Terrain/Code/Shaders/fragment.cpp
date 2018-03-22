#version 330 core

#define colorR 0
#define colorG 1
#define colorB 2
#define minSlope 3
#define maxSlope 4
#define minHeight 5
#define maxHeight 6
#define sizeMultiplier 7
#define heightFade 8
#define slopeFade 9
#define hasTexture 10

struct ConditionalTexture
{
    lowp sampler2D diffuse;
    lowp sampler2D heightmap;
	float[11] data;
};

in lowp float dist;
in lowp vec2 UV;
in lowp float poliDensity;

uniform lowp vec3 global_light_direction;

uniform lowp float ambient_min;
uniform lowp float max_height;
uniform lowp float water_height;
uniform lowp vec3 water_color;
uniform lowp vec3 fog_color;
uniform lowp float fog_distance;
uniform int render_chunk_borders;
uniform int render_heightmap;
uniform int render_light;
uniform unsigned int maxDensity;

uniform lowp sampler2D heightmap;

uniform ConditionalTexture textures[10];

out vec4 color;

float SteppedScalar(float scalar, int nSteps)
{
    return (int(scalar * nSteps) % nSteps) / float(nSteps);
}

vec3 ScalarToColor(float s)
{
    float r = (1 - (s * 0.5));
    r = SteppedScalar(r, 10);
    float g = (s);
    g = SteppedScalar(g, 10);
    float b = 0.f;

    return vec3(r, g, b);
}

void main()
{
    lowp vec4 heightmapVal = texture(heightmap, UV);
    lowp vec3 norm = vec3(heightmapVal.x * 2.f - 1.f, heightmapVal.y * 2.f - 1.f, heightmapVal.z * 2.f - 1.f);
    lowp float height = heightmapVal.w * max_height;
    lowp float slope = 1.f - norm.y;

    lowp vec3 col;

    if (render_heightmap == 0)
    {
		float currentH = 0;
        for (int n = 9; n >= 0; n--)
        {
            if (height >= textures[n].data[minHeight] - textures[n].data[heightFade]
                && height < textures[n].data[maxHeight] + textures[n].data[heightFade]
                && slope >= textures[n].data[minSlope] - textures[n].data[slopeFade]
                && slope <= textures[n].data[maxSlope] + textures[n].data[slopeFade])
            {
				float h = texture(textures[n].heightmap, UV * int(textures[n].data[sizeMultiplier])).x;
				//texture is in it's range
				if (height >= textures[n].data[minHeight]
					&& height < textures[n].data[maxHeight]
					&& slope >= textures[n].data[minSlope]
					&& slope <= textures[n].data[maxSlope])
				{
					currentH = h;
					col = vec3(textures[n].data[colorR], textures[n].data[colorG], textures[n].data[colorB]);
					if (textures[n].data[hasTexture] != 0.f)
					{
						col *= texture(textures[n].diffuse, UV * int(textures[n].data[sizeMultiplier])).xyz;
					}
				}
				//Texture is in fade range
				else
				{
					if (height > textures[n].data[maxHeight])
					{
						float dif = 1- (height - textures[n].data[maxHeight]) / textures[n].data[heightFade];
						h *= dif;
					}
					else if (height < textures[n].data[minHeight])
					{
						float dif = 1- (textures[n].data[minHeight] * height) / textures[n].data[heightFade];
						h *= dif;
					}
					if (h > currentH)
					{
						currentH = h;
						col = vec3(textures[n].data[colorR], textures[n].data[colorG], textures[n].data[colorB]);
						if (textures[n].data[hasTexture] != 0.f)
						{
							col *= texture(textures[n].diffuse, UV * int(textures[n].data[sizeMultiplier])).xyz;
						}
					}
				}
            }
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
    else
    {
        col = ScalarToColor(poliDensity / maxDensity);
    }

    color = vec4(mix(col, fog_color, min((dist*dist) / (fog_distance*fog_distance), 1.f)), 1.f);
}

		/*
		//Water
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
		*/