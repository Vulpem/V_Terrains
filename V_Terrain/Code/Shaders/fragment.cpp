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

uniform lowp vec3 global_light_direction;
uniform lowp mat4 model_matrix;

uniform lowp float ambient_min;
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

//https://freepbr.com/
//https://www.textures.com/download/substance0121/131965
//https://www.textures.com/browse/substance/114546
//https://www.textures.com/download/substance0058/128242

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
    norm.y /= model_matrix[1][1];
	norm = normalize(norm);

    lowp float slope = 1.f - norm.y;
	lowp float height = heightmapVal.w;

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
    
	/*if(renderDensity)
    {
        col = ScalarToColor(poliDensity / maxDensity);
    }*/

    color = vec4(mix(col, fog_color, min((dist*dist) / (fog_distance*fog_distance), 1.f)), 1.f);
}
