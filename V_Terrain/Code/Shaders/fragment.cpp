#version 410 core

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

#define TexNVariables 11
#define nTextures 10

uniform float[nTextures *TexNVariables] textures;

uniform sampler2D diff_0;
uniform sampler2D hm_0;

uniform sampler2D diff_1;
uniform sampler2D hm_1;

uniform sampler2D diff_2;
uniform sampler2D hm_2;

uniform sampler2D diff_3;
uniform sampler2D hm_3;

uniform sampler2D diff_4;
uniform sampler2D hm_4;

uniform sampler2D diff_5;
uniform sampler2D hm_5;

uniform sampler2D diff_6;
uniform sampler2D hm_6;

uniform sampler2D diff_7;
uniform sampler2D hm_7;

uniform sampler2D diff_8;
uniform sampler2D hm_8;

uniform sampler2D diff_9;
uniform sampler2D hm_9;

in vec3 pos;
in vec2 UV;

uniform  vec3 global_light_direction;
uniform  mat4 model_matrix;

uniform  float ambient_min;
uniform  float water_height;
uniform  vec3 water_color;
uniform  vec3 fog_color;
uniform  float fog_distance;
uniform int render_chunk_borders;
uniform int render_heightmap;
uniform int render_light;
uniform int heightmapResolution;

uniform  sampler2D heightmap;

out vec4 color;

vec4 SampleDiff(int texN, vec2 UV)
{
	switch (texN)
	{
	case 0: return texture2D(diff_0, UV);
	case 1: return texture2D(diff_1, UV);
	case 2: return texture2D(diff_2, UV);
	case 3: return texture2D(diff_3, UV);
	case 4: return texture2D(diff_4, UV);
	case 5: return texture2D(diff_5, UV);
	case 6: return texture2D(diff_6, UV);
	case 7: return texture2D(diff_7, UV);
	case 8: return texture2D(diff_8, UV);
	case 9: return texture2D(diff_9, UV);
	default: return vec4(0.f, 0.f, 0.f, 0.f);
	}
}

vec4 SampleHM(int texN, vec2 UV)
{
	switch (texN)
	{
	case 0: return texture2D(hm_0, UV);
	case 1: return texture2D(hm_1, UV);
	case 2: return texture2D(hm_2, UV);
	case 3: return texture2D(hm_3, UV);
	case 4: return texture2D(hm_4, UV);
	case 5: return texture2D(hm_5, UV);
	case 6: return texture2D(hm_6, UV);
	case 7: return texture2D(hm_7, UV);
	case 8: return texture2D(hm_8, UV);
	case 9: return texture2D(hm_9, UV);
	default: return vec4(0.f, 0.f, 0.f, 0.f);
	}
}

float GetVal(int texN, int val)
{
	return textures[texN * TexNVariables + val];
}

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

vec4 GetTerrainColor()
{
	vec4 hmVal = texture2D(heightmap, UV);
	vec3 norm = hmVal.xyz * 2.f - 1.f;

	norm.y /= (model_matrix[1][1] * heightmapResolution)/(model_matrix[0][0]);
	norm = normalize(norm);

	float slope = 1.f - norm.y;
	float height = pos.y / model_matrix[1][1];

	vec3 col = vec3(0.f, 0.f, 0.f);

	if (render_heightmap == 0)
	{
		float currentH = 0;
		for (int n = 9; n >= 0; n--)
		{
			if (height > GetVal(n, minHeight) - GetVal(n, heightFade)
				&& height <= GetVal(n, maxHeight) + GetVal(n, heightFade)
				&& slope > GetVal(n, minSlope) - GetVal(n, slopeFade)
				&& slope <= GetVal(n, maxSlope) + GetVal(n, slopeFade))
			{
				float h = SampleDiff(n, UV * int(GetVal(n, sizeMultiplier))).x;
				//texture is in it's range
				if (height > GetVal(n, minHeight)
					&& height <= GetVal(n, maxHeight)
					&& slope > GetVal(n, minSlope)
					&& slope <= GetVal(n, maxSlope))
				{
					currentH = h;
					col = vec3(GetVal(n, colorR), GetVal(n, colorG), GetVal(n, colorB));
					if (GetVal(n, hasTexture) != 0.f)
					{
						col *= SampleDiff(n, UV * int(GetVal(n, sizeMultiplier))).xyz;
					}
				}
				//Texture is in fade range
				else
				{
					if (height > GetVal(n, maxHeight))
					{
						float dif = 1 - (height - GetVal(n, maxHeight)) / GetVal(n, heightFade);
						h *= dif;
					}
					else if (height < GetVal(n, minHeight))
					{
						float dif = 1 - (GetVal(n, minHeight) * height) / GetVal(n, heightFade);
						h *= dif;
					}
					if (h > currentH)
					{
						currentH = h;
						col = vec3(GetVal(n, colorR), GetVal(n, colorG), GetVal(n, colorB));
						if (GetVal(n, hasTexture) != 0.f)
						{
							col *= SampleDiff(n, UV * int(GetVal(n, sizeMultiplier))).xyz;
						}
					}
				}
			}
		}
	}
	else
	{
		col = vec3(hmVal.w, hmVal.w, hmVal.w);
	}

	if (render_chunk_borders != 0 && (UV.x <= 0.03f || UV.y <= 0.03f || UV.x >= 0.97 || UV.y >= 0.97))
	{
		col = vec3(1.f, 1.f, 1.f);
	}

	if (render_light != 0)
	{
		col *= max(dot(global_light_direction, norm), ambient_min);
	}

	float distanceFog = (gl_FragCoord.z / gl_FragCoord.w) / fog_distance;
	distanceFog = min(distanceFog, 1);

	float heightFog = (1 + water_height - height);
	heightFog = pow(heightFog, 3);

	float fog = distanceFog + (heightFog * 0.5f) * pow(distanceFog, 0.25f);
	fog = min(fog, 1);

	return vec4(mix(col, fog_color, fog), 1.f);
}

void main()
{
	color = GetTerrainColor();
}