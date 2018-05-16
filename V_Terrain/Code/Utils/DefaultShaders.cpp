//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with V Terrains.  If not, see <http://www.gnu.org/licenses/>.
#include "Shaders.h"

#include "../Globals.h"
#include "../ExternalLibs/Glew/include/glew.h"

#include <fstream>

namespace VTerrain
{
    std::string Shaders::GetDefaultVertexShader()
    {
#if _DEBUG
        std::string ret = OpenFile("vertex.cpp");
        if (ret.length() > 1) { return ret; }
#endif
        return std::string("\
            #version 330 core\
\
            in  vec3 position;\
        in  vec2 texCoord;\
\
        uniform  mat4 model_matrix;\
        uniform  sampler2D heightmap;\
        uniform  float water_height;\
\
\
        out vec3 pos;\
        out vec3 norm;\
        out vec2 UV;\
\
        void main()\
        {\
            vec4 hmVal = texture2D(heightmap, texCoord);\
            pos = (model_matrix * vec4(position + vec3(0, hmVal.w, 0), 1.f)).xyz;\
\
            float realWaterHeight = water_height * model_matrix[1][1];\
            if (pos.y < realWaterHeight)\
            {\
                pos.y = realWaterHeight;\
                norm = vec3(0.f, 1.f, 0.f);\
            }\
            else\
            {\
                norm = hmVal.xyz * 2.f - 1.f;\
            }\
            UV = texCoord;\
        }\
            ");
    }

    std::string Shaders::GetDefaultFragmentShader()
    {
#if _DEBUG
        std::string ret = OpenFile("fragment.cpp");
        if (ret.length() > 1) { return ret; }
#endif
        return std::string("\
            #version 330 core\
\
#define colorR 0\
#define colorG 1\
#define colorB 2\
#define minSlope 3\
#define maxSlope 4\
#define minHeight 5\
#define maxHeight 6\
#define sizeMultiplier 7\
#define heightFade 8\
#define slopeFade 9\
#define hasTexture 10\
\
            struct ConditionalTexture\
        {\
            sampler2D diffuse;\
            sampler2D heightmap;\
            float[11] data;\
        };\
\
        in vec3 pos;\
        in vec3 norm;\
        in vec2 UV;\
\
        uniform  vec3 global_light_direction;\
        uniform  mat4 model_matrix;\
\
        uniform  float ambient_min;\
        uniform  float water_height;\
        uniform  vec3 water_color;\
        uniform  vec3 fog_color;\
        uniform  float fog_distance;\
        uniform int render_chunk_borders;\
        uniform int render_heightmap;\
        uniform int render_light;\
        uniform unsigned int maxDensity;\
\
        uniform  sampler2D heightmap;\
\
        uniform ConditionalTexture textures[10];\
\
        out vec4 color;\
\
        //https://freepbr.com/\
        //https://www.textures.com/download/substance0121/131965\
        //https://www.textures.com/browse/substance/114546\
        //https://www.textures.com/download/substance0058/128242\
\
        float SteppedScalar(float scalar, int nSteps)\
        {\
            return (int(scalar * nSteps) % nSteps) / float(nSteps);\
        }\
\
        vec3 ScalarToColor(float s)\
        {\
            float r = (1 - (s * 0.5));\
            r = SteppedScalar(r, 10);\
            float g = (s);\
            g = SteppedScalar(g, 10);\
            float b = 0.f;\
\
            return vec3(r, g, b);\
        }\
\
        void main()\
        {\
            vec3 n = norm;\
            n.y /= model_matrix[1][1];\
            n = normalize(n);\
\
            float slope = 1.f - n.y;\
            float height = pos.y / model_matrix[1][1];\
\
            vec3 col;\
\
            if (render_heightmap == 0)\
            {\
                float currentH = 0;\
                for (int n = 9; n >= 0; n--)\
                {\
                    if (height >= textures[n].data[minHeight] - textures[n].data[heightFade]\
                        && height < textures[n].data[maxHeight] + textures[n].data[heightFade]\
                        && slope >= textures[n].data[minSlope] - textures[n].data[slopeFade]\
                        && slope <= textures[n].data[maxSlope] + textures[n].data[slopeFade])\
                    {\
                        float h = texture(textures[n].heightmap, UV * int(textures[n].data[sizeMultiplier])).x;\
                        //texture is in it's range\
                        if (height >= textures[n].data[minHeight]\
                            && height < textures[n].data[maxHeight]\
                            && slope >= textures[n].data[minSlope]\
                            && slope <= textures[n].data[maxSlope])\
                        {\
                            currentH = h;\
                            col = vec3(textures[n].data[colorR], textures[n].data[colorG], textures[n].data[colorB]);\
                            if (textures[n].data[hasTexture] != 0.f)\
                            {\
                                col *= texture(textures[n].diffuse, UV * int(textures[n].data[sizeMultiplier])).xyz;\
                            }\
                        }\
                        //Texture is in fade range\
                        else\
                        {\
                            if (height > textures[n].data[maxHeight])\
                            {\
                                float dif = 1 - (height - textures[n].data[maxHeight]) / textures[n].data[heightFade];\
                                h *= dif;\
                            }\
                            else if (height < textures[n].data[minHeight])\
                            {\
                                float dif = 1 - (textures[n].data[minHeight] * height) / textures[n].data[heightFade];\
                                h *= dif;\
                            }\
                            if (h > currentH)\
                            {\
                                currentH = h;\
                                col = vec3(textures[n].data[colorR], textures[n].data[colorG], textures[n].data[colorB]);\
                                if (textures[n].data[hasTexture] != 0.f)\
                                {\
                                    col *= texture(textures[n].diffuse, UV * int(textures[n].data[sizeMultiplier])).xyz;\
                                }\
                            }\
                        }\
                    }\
                }\
            }\
            else\
            {\
                float val = texture(heightmap, UV).w;\
                col = vec3(val, val, val);\
            }\
\
            if (render_chunk_borders != 0 && (UV.x <= 0.01f || UV.y <= 0.01f || UV.x >= 0.99 || UV.y >= 0.99))\
            {\
                col = vec3(1.f, 1.f, 1.f);\
            }\
\
            if (render_light != 0)\
            {\
                col *= max(dot(global_light_direction, n), ambient_min);\
            }\
\
            /*if(renderDensity)\
            {\
            col = ScalarToColor(poliDensity / maxDensity);\
            }*/\
\
            float fogStrength = (gl_FragCoord.z / gl_FragCoord.w) / fog_distance;\
            fogStrength = min(fogStrength, 1);\
\
            color = vec4(mix(col, fog_color, fogStrength), 1.f);\
        }\
            ");
    }

    std::string Shaders::GetDefaultTCSShader()
    {
#if _DEBUG
        std::string ret = OpenFile("TCS.cpp");
        if (ret.length() > 1) { return ret; }
#endif
        return std::string("\
            #version 430 core\
\
            //TODO send as uniforms\
            float triSize = 20.f;\
        vec2 viewport = vec2(1920, 1080);\
\
\
        layout(vertices = 4) out;\
\
        uniform  mat4 view_matrix;\
        uniform  mat4 projection_matrix;\
\
        in vec3 pos[];\
        in vec3 norm[];\
        in vec2 UV[];\
\
        out vec3 position[];\
        out vec3 normal[];\
        out vec2 UVs[];\
\
        //https://developer.nvidia.com/gameworks-vulkan-and-opengl-samples\
\
        vec4 worldToScreen(vec3 p)\
        {\
            vec4 r = (projection_matrix * view_matrix) * vec4(p, 1.0);   // to clip space\
            r.xyz /= r.w;            // project\
            r.xyz = r.xyz*0.5 + 0.5;  // to NDC\
            return r;\
        }\
\
        vec4 eyeToScreen(vec4 p)\
        {\
            vec4 r = projection_matrix * p;   // to clip space\
            r.xyz /= r.w;            // project\
            r.xyz = r.xyz*0.5 + 0.5;  // to NDC\
            return r;\
        }\
\
        bool sphereInScreen(vec3 pos, float allowance)\
        {\
            vec4 p = worldToScreen(pos);\
            return (\
                p.x > 0.0f - allowance && p.x < 1.f + allowance\
                && p.y > 0.0f - allowance && p.y < 1.f + allowance\
                && p.z > 0.0f);\
        }\
\
        float calcEdgeTessellation(vec2 s0, vec2 s1)\
        {\
            float d = distance(s0 * viewport, s1 * viewport);\
            return clamp(d / triSize, 1, 61);\
        }\
\
        float calcEdgeTessellationSphere(vec3 w0, vec3 w1, float diameter)\
        {\
            vec3 centre = (w0 + w1) * 0.5;\
            vec4 view0 = (view_matrix)* vec4(centre, 1.0);\
            vec4 view1 = view0 + vec4(diameter, 0, 0, 0);\
            vec2 s0 = eyeToScreen(view0).xy;\
            vec2 s1 = eyeToScreen(view1).xy;\
            return calcEdgeTessellation(s0, s1);\
        }\
\
        void main()\
        {\
            position[gl_InvocationID] = pos[gl_InvocationID];\
            normal[gl_InvocationID] = norm[gl_InvocationID];\
            UVs[gl_InvocationID] = UV[gl_InvocationID];\
\
            vec3 spherePos = (pos[0] + pos[1] + pos[2] + pos[3]) *0.25f;\
            float sphereDiam = distance(spherePos, pos[0]) * 1.1f;\
            vec2 diamOnScreen = eyeToScreen(vec4(sphereDiam, 0.f, 0.f, 1.f)).xy;\
\
            if (!sphereInScreen(spherePos, 0.15))//length(diamOnScreen) / 2.f))\
            {\
                gl_TessLevelOuter[0] = -1;\
                gl_TessLevelOuter[1] = -1;\
                gl_TessLevelOuter[2] = -1;\
                gl_TessLevelOuter[3] = -1;\
\
                gl_TessLevelInner[0] = -1;\
                gl_TessLevelInner[1] = -1;\
            }\
            else\
            {\
                gl_TessLevelOuter[0] = calcEdgeTessellationSphere(pos[3], pos[0], sphereDiam);\
                gl_TessLevelOuter[1] = calcEdgeTessellationSphere(pos[0], pos[1], sphereDiam);\
                gl_TessLevelOuter[2] = calcEdgeTessellationSphere(pos[1], pos[2], sphereDiam);\
                gl_TessLevelOuter[3] = calcEdgeTessellationSphere(pos[2], pos[3], sphereDiam);\
\
                gl_TessLevelInner[0] = 0.5 * (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]);\
                gl_TessLevelInner[1] = 0.5 * (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]);\
            }\
        }\
            ");
    }

    std::string Shaders::GetDefaultTESShader()
    {
#if _DEBUG
        std::string ret = OpenFile("TES.cpp");
        if (ret.length() > 1) { return ret; }
#endif
        return std::string("\
            #version 430 core\
\
            layout(quads, ccw, fractional_even_spacing) in;\
\
        in vec3 position[];\
        in vec3 normal[];\
        in vec2 UVs[];\
\
        out vec3 pos;\
        out vec3 norm;\
        out vec2 UV;\
\
        uniform  mat4 view_matrix;\
        uniform  mat4 projection_matrix;\
        uniform  mat4 model_matrix;\
\
        uniform  float fog_distance;\
        uniform  float water_height;\
\
        uniform  sampler2D heightmap;\
\
        void main(void)\
        {\
            UV = mix(mix(UVs[0], UVs[1], gl_TessCoord.x), mix(UVs[3], UVs[2], gl_TessCoord.x), gl_TessCoord.y);\
            vec3 position = mix(mix(position[0], position[1], gl_TessCoord.x), mix(position[3], position[2], gl_TessCoord.x), gl_TessCoord.y).xyz;\
            norm = mix(mix(normal[0], normal[1], gl_TessCoord.x), mix(normal[3], normal[2], gl_TessCoord.x), gl_TessCoord.y);\
\
            pos = position;\
            gl_Position = projection_matrix * view_matrix * vec4(position, 1);\
        }\
            ");
    }
}