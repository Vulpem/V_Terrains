//  RPG Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "RPG Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "RPG Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with RPG Terrains.  If not, see <http://www.gnu.org/licenses/>.
#include "Shaders.h"

#include "../Globals.h"
#include "../ExternalLibs/Glew/include/glew.h"

#include <fstream>

namespace RPGT
{
    std::string Shaders::GetDefaultVertexShader()
    {
#if _DEBUG
        std::string ret = OpenFile("vertex.cpp");
        if (ret.length() > 1) { return ret; }
#endif
        return std::string(
"#version 330 core\n\
\n\
in  vec3 position;\n\
        in  vec2 texCoord;\n\
\n\
        uniform  mat4 model_matrix;\n\
        uniform  sampler2D heightmap;\n\
        uniform  float water_height;\n\
\n\
\n\
        out vec3 pos;\n\
        out vec2 UV;\n\
\n\
        void main()\n\
        {\n\
            pos = position;\n\
            UV = texCoord;\n\
        }\n\
        ");
    }

    std::string Shaders::GetDefaultFragmentShader() 
    {            
#if _DEBUG       
        std::string ret = OpenFile("fragment.cpp"); 
        if (ret.length() > 1) { return ret; }       
#endif           
        return std::string(              
"#version 410 core\n\
\n\
#define colorR 0\n\
#define colorG 1\n\
#define colorB 2\n\
#define minSlope 3\n\
#define maxSlope 4\n\
#define minHeight 5\n\
#define maxHeight 6\n\
#define sizeMultiplier 7\n\
#define heightFade 8\n\
#define slopeFade 9\n\
#define hasTexture 10\n\
\n\
#define TexNVariables 11\n\
#define nTextures 10\n\
\n\
uniform float[nTextures *TexNVariables] textures;\n\
\n\
		uniform sampler2D diff_0;\n\
		uniform sampler2D hm_0;\n\
\n\
		uniform sampler2D diff_1;\n\
		uniform sampler2D hm_1;\n\
\n\
		uniform sampler2D diff_2;\n\
		uniform sampler2D hm_2;\n\
\n\
		uniform sampler2D diff_3;\n\
		uniform sampler2D hm_3;\n\
\n\
		uniform sampler2D diff_4;\n\
		uniform sampler2D hm_4;\n\
\n\
		uniform sampler2D diff_5;\n\
		uniform sampler2D hm_5;\n\
\n\
		uniform sampler2D diff_6;\n\
		uniform sampler2D hm_6;\n\
\n\
		uniform sampler2D diff_7;\n\
		uniform sampler2D hm_7;\n\
\n\
		uniform sampler2D diff_8;\n\
		uniform sampler2D hm_8;\n\
\n\
		uniform sampler2D diff_9;\n\
		uniform sampler2D hm_9;\n\
\n\
		in vec3 pos;\n\
		in vec2 UV;\n\
\n\
		uniform  vec3 global_light_direction;\n\
		uniform  mat4 model_matrix;\n\
\n\
		uniform  float ambient_min;\n\
		uniform  float water_height;\n\
		uniform  vec3 water_color;\n\
		uniform  vec3 fog_color;\n\
		uniform  float fog_distance;\n\
		uniform int render_chunk_borders;\n\
		uniform int render_heightmap;\n\
		uniform int render_light;\n\
		uniform unsigned int maxDensity;\n\
\n\
		uniform  sampler2D heightmap;\n\
\n\
		out vec4 color;\n\
\n\
		vec4 SampleDiff(int texN, vec2 UV)\n\
		{\n\
			switch (texN)\n\
			{\n\
			case 0: return texture2D(diff_0, UV);\n\
			case 1: return texture2D(diff_1, UV);\n\
			case 2: return texture2D(diff_2, UV);\n\
			case 3: return texture2D(diff_3, UV);\n\
			case 4: return texture2D(diff_4, UV);\n\
			case 5: return texture2D(diff_5, UV);\n\
			case 6: return texture2D(diff_6, UV);\n\
			case 7: return texture2D(diff_7, UV);\n\
			case 8: return texture2D(diff_8, UV);\n\
			case 9: return texture2D(diff_9, UV);\n\
			default: return vec4(0.f, 0.f, 0.f, 0.f);\n\
			}\n\
		}\n\
\n\
		vec4 SampleHM(int texN, vec2 UV)\n\
		{\n\
			switch (texN)\n\
			{\n\
			case 0: return texture2D(hm_0, UV);\n\
			case 1: return texture2D(hm_1, UV);\n\
			case 2: return texture2D(hm_2, UV);\n\
			case 3: return texture2D(hm_3, UV);\n\
			case 4: return texture2D(hm_4, UV);\n\
			case 5: return texture2D(hm_5, UV);\n\
			case 6: return texture2D(hm_6, UV);\n\
			case 7: return texture2D(hm_7, UV);\n\
			case 8: return texture2D(hm_8, UV);\n\
			case 9: return texture2D(hm_9, UV);\n\
			default: return vec4(0.f, 0.f, 0.f, 0.f);\n\
			}\n\
		}\n\
\n\
		float GetVal(int texN, int val)\n\
		{\n\
			return textures[texN * TexNVariables + val];\n\
		}\n\
\n\
		float SteppedScalar(float scalar, int nSteps)\n\
		{\n\
			return (int(scalar * nSteps) % nSteps) / float(nSteps);\n\
		}\n\
\n\
		vec3 ScalarToColor(float s)\n\
		{\n\
			float r = (1 - (s * 0.5));\n\
			r = SteppedScalar(r, 10);\n\
			float g = (s);\n\
			g = SteppedScalar(g, 10);\n\
			float b = 0.f;\n\
\n\
			return vec3(r, g, b);\n\
		}\n\
\n\
		vec4 GetTerrainColor()\n\
		{\n\
			vec4 hmVal = texture2D(heightmap, UV);\n\
			vec3 norm = hmVal.xyz * 2.f - 1.f;\n\
\n\
			norm.y /= model_matrix[1][1];\n\
			norm = normalize(norm);\n\
\n\
			float slope = 1.f - norm.y;\n\
			float height = pos.y / model_matrix[1][1];\n\
\n\
			vec3 col = vec3(0.f, 0.f, 0.f);\n\
\n\
			if (render_heightmap == 0)\n\
			{\n\
				float currentH = 0;\n\
				for (int n = 9; n >= 0; n--)\n\
				{\n\
					if (height > GetVal(n, minHeight) - GetVal(n, heightFade)\
						&& height <= GetVal(n, maxHeight) + GetVal(n, heightFade)\n\
						&& slope > GetVal(n, minSlope) - GetVal(n, slopeFade)\
						&& slope <= GetVal(n, maxSlope) + GetVal(n, slopeFade))\
					{\n\
						float h = SampleDiff(n, UV * int(GetVal(n, sizeMultiplier))).x;\n\
						//texture is in it's range\n\
						if (height > GetVal(n, minHeight)\n\
							&& height <= GetVal(n, maxHeight)\n\
							&& slope > GetVal(n, minSlope)\n\
							&& slope <= GetVal(n, maxSlope))\n\
						{\n\
							currentH = h;\n\
							col = vec3(GetVal(n, colorR), GetVal(n, colorG), GetVal(n, colorB));\n\
							if (GetVal(n, hasTexture) != 0.f)\n\
							{\n\
								col *= SampleDiff(n, UV * int(GetVal(n, sizeMultiplier))).xyz;\n\
							}\n\
						}\n\
						//Texture is in fade range\n\
						else\n\
						{\n\
							if (height > GetVal(n, maxHeight))\n\
							{\n\
								float dif = 1 - (height - GetVal(n, maxHeight)) / GetVal(n, heightFade);\n\
								h *= dif;\n\
							}\n\
							else if (height < GetVal(n, minHeight))\n\
							{\n\
								float dif = 1 - (GetVal(n, minHeight) * height) / GetVal(n, heightFade);\n\
								h *= dif;\n\
							}\n\
							if (h > currentH)\n\
							{\n\
								currentH = h;\n\
								col = vec3(GetVal(n, colorR), GetVal(n, colorG), GetVal(n, colorB));\n\
								if (GetVal(n, hasTexture) != 0.f)\n\
								{\n\
									col *= SampleDiff(n, UV * int(GetVal(n, sizeMultiplier))).xyz;\n\
								}\n\
							}\n\
						}\n\
					}\n\
				}\n\
			}\n\
			else\n\
			{\n\
				col = vec3(hmVal.w, hmVal.w, hmVal.w);\n\
			}\n\
\n\
			if (render_chunk_borders != 0 && (UV.x <= 0.03f || UV.y <= 0.03f || UV.x >= 0.97 || UV.y >= 0.97))\n\
			{\n\
				col = vec3(1.f, 1.f, 1.f);\n\
			}\n\
\n\
			if (render_light != 0)\n\
			{\n\
				col *= max(dot(global_light_direction, norm), ambient_min);\n\
			}\n\
\n\
			/*if(renderDensity)\n\
			{\n\
			col = ScalarToColor(poliDensity / maxDensity);\n\
			}*/\n\
\n\
			float distanceFog = (gl_FragCoord.z / gl_FragCoord.w) / fog_distance;\n\
			distanceFog = min(distanceFog, 1);\n\
\n\
			float heightFog = (1 + water_height - height);\n\
			heightFog = pow(heightFog, 3);\n\
\n\
			float fog = distanceFog + (heightFog * 0.5f) * pow(distanceFog, 0.25f);\n\
			fog = min(fog, 1);\n\
\n\
			return vec4(mix(col, fog_color, fog), 1.f);\n\
		}\n\
\n\
		void main()\n\
		{\n\
			color = GetTerrainColor();\n\
		}"
        );
    }

    std::string Shaders::GetDefaultTCSShader()
    {
#if _DEBUG
        std::string ret = OpenFile("TCS.cpp");
        if (ret.length() > 1) { return ret; }
#endif
        return std::string(
"#version 430 core\n\
\n\
//TODO send as uniforms\n\
float triSize = 6.5f;\n\
        vec2 viewport = vec2(1920, 1080);\n\
\n\
\n\
        layout(vertices = 4) out;\n\
\n\
        uniform mat4 model_matrix;\n\
        uniform  mat4 view_matrix;\n\
        uniform  mat4 projection_matrix;\n\
\n\
        in vec3 pos[];\n\
        in vec2 UV[];\n\
\n\
        out vec3 position[];\n\
        out vec2 UVs[];\n\
\n\
        //https://developer.nvidia.com/gameworks-vulkan-and-opengl-samples\n\
\n\
        vec4 worldToScreen(vec3 p)\n\
        {\n\
            vec4 r = (projection_matrix * view_matrix) * vec4(p, 1.0);   // to clip space\n\
            r.xyz /= r.w;            // project\n\
            r.xyz = r.xyz*0.5 + 0.5;  // to NDC\n\
            return r;\n\
        }\n\
\n\
        vec4 eyeToScreen(vec4 p)\n\
        {\n\
            vec4 r = projection_matrix * p;   // to clip space\n\
            r.xyz /= r.w;            // project\n\
            r.xyz = r.xyz*0.5 + 0.5;  // to NDC\n\
            return r;\n\
        }\n\
\n\
        bool sphereInScreen(vec3 pos, float allowance)\n\
        {\n\
            vec4 p = worldToScreen(pos);\n\
            return (\n\
                (p.x > 0.0f - allowance && p.x < 1.f + allowance)\n\
                // && p.y > 0.0f - allowance && p.y < 1.f + allowance\n\
                || p.z < 0.995);\n\
        }\n\
\n\
        float calcEdgeTessellation(vec2 s0, vec2 s1)\n\
        {\n\
            float d = distance(s0 * viewport, s1 * viewport);\n\
            return clamp(d / triSize, 1, 61);\n\
        }\n\
\n\
        float calcEdgeTessellationSphere(vec3 w0, vec3 w1, float diameter)\n\
        {\n\
            vec3 centre = (w0 + w1) * 0.5;\n\
            vec4 view0 = (view_matrix)* vec4(centre, 1.0);\n\
            vec4 view1 = view0 + vec4(diameter, 0, 0, 0);\n\
            vec2 s0 = eyeToScreen(view0).xy;\n\
            vec2 s1 = eyeToScreen(view1).xy;\n\
            return calcEdgeTessellation(s0, s1);\n\
        }\n\
\n\
        int calcPowEdgeTessellationSphere(vec3 w0, vec3 w1, float diameter)\n\
        {\n\
            int tess = int(floor(calcEdgeTessellationSphere(w0, w1, diameter)));\n\
            int n = 1;\n\
            while (tess > int(pow(2, n)))\n\
            {\n\
\n\
                n++;\n\
            }\n\
\n\
            return int(pow(2, n - 1));\n\
        }\n\
\n\
        void main()\n\
        {\n\
            position[gl_InvocationID] = pos[gl_InvocationID];\n\
            UVs[gl_InvocationID] = UV[gl_InvocationID];\n\
\n\
            vec3 p[4] = {\n\
                (model_matrix * vec4(pos[0], 1.f)).xyz,\n\
                (model_matrix * vec4(pos[1], 1.f)).xyz,\n\
                (model_matrix * vec4(pos[2], 1.f)).xyz,\n\
                (model_matrix * vec4(pos[3], 1.f)).xyz\n\
            };\n\
            vec3 spherePos = (p[0] + p[1] + p[2] + p[3]) *0.25f;\n\
            float sphereDiam = distance(spherePos, p[0]) * 1.1f;\n\
            vec2 diamOnScreen = eyeToScreen(vec4(sphereDiam, 0.f, 0.f, 1.f)).xy;\n\
\n\
            if (!sphereInScreen(spherePos, 0.15))//length(diamOnScreen) / 2.f))\n\
            {\n\
                gl_TessLevelOuter[0] = -1;\n\
                gl_TessLevelOuter[1] = -1;\n\
                gl_TessLevelOuter[2] = -1;\n\
                gl_TessLevelOuter[3] = -1;\n\
\n\
                gl_TessLevelInner[0] = -1;\n\
                gl_TessLevelInner[1] = -1;\n\
            }\n\
            else\n\
            {\n\
                gl_TessLevelOuter[0] = calcPowEdgeTessellationSphere(p[3], p[0], sphereDiam);\n\
                gl_TessLevelOuter[1] = calcPowEdgeTessellationSphere(p[0], p[1], sphereDiam);\n\
                gl_TessLevelOuter[2] = calcPowEdgeTessellationSphere(p[1], p[2], sphereDiam);\n\
                gl_TessLevelOuter[3] = calcPowEdgeTessellationSphere(p[2], p[3], sphereDiam);\n\
\n\
                gl_TessLevelInner[1] = gl_TessLevelInner[0] = gl_TessLevelOuter[0];\n\
            }\n\
        }\n\
            ");
    }

    std::string Shaders::GetDefaultTESShader()
    {
#if _DEBUG
        std::string ret = OpenFile("TES.cpp");
        if (ret.length() > 1) { return ret; }
#endif
        return std::string(
"#version 430 core\n\
\n\
layout(quads, ccw) in;\n\
\n\
        in vec3 position[];\n\
        in vec2 UVs[];\n\
\n\
        out vec3 pos;\n\
        out vec2 UV;\n\
\n\
        uniform  mat4 view_matrix;\n\
        uniform  mat4 projection_matrix;\n\
        uniform  mat4 model_matrix;\n\
\n\
        uniform  float fog_distance;\n\
        uniform  float water_height;\n\
\n\
        uniform  sampler2D heightmap;\n\
\n\
        void main(void)\n\
        {\n\
            UV = mix(mix(UVs[0], UVs[1], gl_TessCoord.x), mix(UVs[3], UVs[2], gl_TessCoord.x), gl_TessCoord.y);\n\
            vec3 position = mix(mix(position[0], position[1], gl_TessCoord.x), mix(position[3], position[2], gl_TessCoord.x), gl_TessCoord.y).xyz;\n\
\n\
            vec4 hmVal = texture2D(heightmap, UV);\n\
            pos = (model_matrix * vec4(position + vec3(0, hmVal.w, 0), 1.f)).xyz;\n\
\n\
            float realWaterHeight = water_height * model_matrix[1][1];\n\
            if (pos.y < realWaterHeight)\n\
            {\n\
                pos.y = realWaterHeight;\n\
            }\n\
\n\
            gl_Position = projection_matrix * view_matrix * vec4(pos, 1);\n\
        }\n\
            ");
    }
}