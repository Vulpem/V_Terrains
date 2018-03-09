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
#include <iostream>

namespace VTerrain
{
    std::string Shaders::m_defaultVertexShader = std::string(
        "#version 330 core\n"
        ""
        "layout (location = 0) in lowp vec3 position;"
        "layout (location = 1) in lowp vec2 texCoord;"
        ""
		"out lowp vec3 pos;"
		"out lowp float fog;"
        "out lowp vec2 UV;"
        ""
        "uniform lowp mat4 view_matrix;"
        "uniform lowp mat4 projection_matrix;"
		""
        "uniform lowp vec3 position_offset;"
		"uniform lowp float max_height;"
		""
        "uniform lowp float fog_distance;"
		"uniform lowp float water_height;"
        ""
        "uniform lowp sampler2D heightmap;"
        ""
        "void main()"
        "{"
        "	lowp mat4 transform = projection_matrix * view_matrix;"
        "   lowp vec4 heightMapVal = texture(heightmap, texCoord);"
		"   pos = position + position_offset + vec3(0, heightMapVal.w * max_height, 0);"
		"   pos.y = max(pos.y, water_height);"
		"   vec4 outPos = transform * vec4(pos,1);"
		"	gl_Position = outPos;"
		""
		"   fog = min((outPos.x * outPos.x + outPos.y * outPos.y + outPos.z * outPos.z)/(fog_distance * fog_distance), 1);"
		""
        "   UV = texCoord;"
        "}"
    );

	std::string Shaders::m_defaultFragmentShader = std::string(
		"#version 330 core\n"
		""
		"in lowp vec3 pos;"
		"in lowp float fog;"
		"in lowp vec2 UV;"
		""
		"uniform lowp vec3 global_light_direction;"
		""
		"uniform lowp float ambient_min;"
		"uniform lowp float max_height;"
		"uniform lowp float water_height;"
		"uniform lowp vec3 water_color;"
		"uniform lowp vec3 fog_color;"
		""
		"uniform lowp sampler2D heightmap;"
		""
		"out vec4 color;"
		""
		"void main()"
		"{"
		""
		"	lowp vec4 heightmapVal = texture(heightmap, UV);"
        "   lowp vec3 norm = vec3(heightmapVal.x * 2.f - 1.f,heightmapVal.y * 2.f - 1.f,heightmapVal.z * 2.f - 1.f);"
		"   lowp float height = heightmapVal.w * max_height;"
		"	lowp float lightIntensity = max(dot(global_light_direction, norm), ambient_min);"
		""
		"   lowp vec3 col;"
		//Water
		"	if (height <= water_height + water_height * 0.0001f)"
		"	{ col = water_color; }"
		//Steep 2
		"	else if(norm.y < 0.1f)"
		"{"
		"	if (height < (max_height - water_height) * 0.62f + water_height)"
		"	{"
		"		col = vec3(0.535f, 0.613f, 0.672f);"
		"	}"
		"	else"
		"	{"
		"		col = vec3(0.335f, 0.413f, 0.472f);"
		"	}"
		"}"
		//Steep
		"	else if(norm.y < 0.15f && height > (max_height - water_height) * 0.56f + water_height)"
		"   { col = vec3(0.435f, 0.513f, 0.572f); }"
		//Sand
		"   else if (height <= water_height + 30)"
		"   { col = vec3(0.949f, 0.823f, 0.662f); }"
		//Snow
		"   else if(height > (max_height - water_height) * 0.62f + water_height)"
		"   { col = vec3(0.937f, 0.981f, 1.f); }"
		//Default
		"   else"
		"   { col = vec3(0.478f, 0.8f, 0.561f); }"
		""
		"   col *= lightIntensity;"
		"   color = vec4(mix(col, fog_color, fog), 1.f);"
        "}"
    );

    Shader Shaders::CompileShader(const char * vertexBuf, const char * fragmentBuf)
    {
        bool error = false;
        GLint success;
        Shader ret;

        unsigned int vertexShader;
        if (vertexBuf == nullptr)
        {
            ret.m_vert_result += "- Using default vertex\n";
            vertexBuf = m_defaultVertexShader.c_str();
        }

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexBuf, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (success == 0)
        {
            error = true;
            GLchar infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            ret.m_vert_result += infoLog;
            ret.m_vert_result += '\n';
			assert(false);
        }
        else
        {
            ret.m_vert_result += "Compilation succesfull\n";
        }

		unsigned int fragmentShader;
        if (fragmentBuf == nullptr)
        {
            ret.m_frag_result += "- Using default fragment\n";
            fragmentBuf = m_defaultFragmentShader.c_str();
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentBuf, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (success == 0)
        {
            error = true;
            GLchar infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            ret.m_frag_result += infoLog;
            ret.m_frag_result += '\n';
			assert(false);
        }
        else
        {
            ret.m_frag_result += "Compilation succesfull\n";
        }

		unsigned int program;
        program = glCreateProgram();

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success == 0)
        {
            error = true;
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            ret.m_program_result += "\n------ Shader Program ------\n";
            ret.m_program_result += infoLog;
            ret.m_program_result += '\n';
        }

        if (program != 0 && error == false)
        {
            ret.m_program = program;

            ret.loc_view_matrix = glGetUniformLocation(program, "view_matrix");
            ret.loc_projection_matrix = glGetUniformLocation(program, "projection_matrix");

            ret.loc_position_offset = glGetUniformLocation(program, "position_offset");
            ret.loc_global_light_direction = glGetUniformLocation(program, "global_light_direction");
            ret.loc_max_height = glGetUniformLocation(program, "max_height");
            ret.loc_fog_distance = glGetUniformLocation(program, "fog_distance");
            ret.loc_fog_color = glGetUniformLocation(program, "fog_color");
            ret.loc_water_color = glGetUniformLocation(program, "water_color");
            ret.loc_water_height = glGetUniformLocation(program, "water_height");
            ret.loc_ambient_color = glGetUniformLocation(program, "ambient_min");

            ret.loc_position = 0;
            ret.loc_texCoord = 1;
        }
        else
        {
            ret.m_program_result += "Error Compiling shader";
            
            assert(false);
        }

        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return ret;
    }

    void Shaders::FreeShader(const Shader& shader)
    {
        glDeleteProgram(shader.m_program);
    }
}