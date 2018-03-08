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
        "layout (location = 0) in vec3 position;"
        "layout (location = 1) in vec2 texCoord;"
        ""
		"out vec3 pos;"
		"out vec3 norm;"
		"out float lightIntensity;"
		"out float fog;"
        "out vec2 UV;"
        ""
        "uniform mat4 model_matrix;"
        "uniform mat4 view_matrix;"
        "uniform mat4 projection_matrix;"
		""
        "uniform vec3 position_offset;"
		"uniform float max_height;"
		""
        "uniform vec3 global_light_direction;"
        "uniform vec4 ambient_color;"
        "uniform float fog_distance;"
		"uniform float water_height;"
        ""
        "uniform sampler2D heightmap;"
        ""
        "void main()"
        "{"
        "	mat4 transform = projection_matrix * view_matrix * model_matrix;"
        "   vec4 heightMapVal = texture(heightmap, texCoord);"
		"   pos = position + position_offset + vec3(0, heightMapVal.x * max_height, 0);"
		"   pos.y = max(pos.y, water_height);"
		"   vec4 outPos = transform * vec4(pos,1);"
		"	gl_Position = outPos;"
		""
        "	norm = mat3(model_matrix) * heightMapVal.yzw;"
        "	lightIntensity = max(dot(global_light_direction, norm), ambient_color.x);"
		""
		"   fog = min((outPos.x * outPos.x + outPos.y * outPos.y + outPos.z * outPos.z)/(fog_distance * fog_distance), 1);"
		""
        "   UV = texCoord;"
        "}"
    );

	std::string Shaders::m_defaultFragmentShader = std::string(
		"#version 330 core\n"
		""
		"in vec3 pos;"
		"in vec4 heightMapVal;"
		"in float lightIntensity;"
		"in float fog;"
		"in vec2 UV;"
		""
		"uniform float max_height;"
		"uniform float fog_distance;"
		"uniform float water_height;"
		"uniform vec3 water_color;"
		"uniform vec3 fog_color;"
		""
		"uniform sampler2D heightmap;"
		""
		"out vec4 color;"
		""
		"void main()"
		"{"
		"   float heightColor = lightIntensity * (pos.y / max_height);"
		"   vec3 col;"
		"	if (pos.y <= water_height + water_height * 0.0001f)"
		"	{ col = water_color; }"
		"   else if (pos.y <= water_height + 30)"
		"   { col = vec3(0.949f, 0.823f, 0.662f); }"
		"   else if(pos.y > max_height * 0.8f)"
		"   { col = vec3(0.937f, 0.981f, 1.f); }"
		"   else"
		"   { col = vec3(0.478f, 0.8f, 0.561f); }"
		"   col *= lightIntensity;"
		"   color = vec4(mix(col, fog_color, fog), 1.f);"
        "}"
    );

    std::string Shaders::CompileShader(const char * vertexBuf, const char * fragmentBuf, unsigned int & shaderProgram)
    {
        std::string ret;
        bool error = false;
        shaderProgram = 0;
        GLint success;

        ret += "\n------ Vertex shader ------\n";
        unsigned int vertexShader;
        if (vertexBuf == nullptr)
        {
            ret += "- Default vertex\n";
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
            ret += infoLog;
            ret += '\n';
			assert(false);
        }
        else
        {
            ret += "Compilation succesfull\n";
        }
		ret.clear();

        ret += "\n------ Fragment shader ------\n";
		unsigned int fragmentShader;
        if (fragmentBuf == nullptr)
        {
            ret += "- Default fragment\n";
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
            ret += infoLog;
            ret += '\n';
			assert(false);
        }
        else
        {
            ret += "Compilation succesfull\n";
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
            ret += "\n------ Shader Program ------\n";
            ret += infoLog;
            ret += '\n';
        }

        if (program != 0 && error == false)
        {
            shaderProgram = program;
        }
        else
        {
            ret += "Error Compiling shader";
            
            assert(false);
        }

        glDetachShader(shaderProgram, vertexShader);
        glDetachShader(shaderProgram, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return ret;
    }

    void Shaders::FreeShader(unsigned int shaderProgram)
    {
        glDeleteProgram(shaderProgram);
    }
}