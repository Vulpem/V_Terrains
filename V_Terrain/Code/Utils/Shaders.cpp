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
        "\n"
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec2 texCoord;\n"
        "\n"
		"out vec3 pos;\n"
		"out vec3 norm;\n"
		"out float lightIntensity;\n"
		"out float dist;\n"
        "\n"
        "uniform mat4 model_matrix;\n"
        "uniform mat4 view_matrix;\n"
        "uniform mat4 projection_matrix;\n"
		"\n"
        "uniform vec3 position_offset;\n"
		"uniform float max_height;\n"
		"\n"
        "uniform vec3 global_light_direction;\n"
        "uniform vec4 ambient_color;\n"
        "\n"
        "uniform sampler2D heightmap;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	mat4 transform = projection_matrix * view_matrix * model_matrix;\n"
        "   vec4 heightMapVal = texture(heightmap, texCoord);\n"
		"   pos = position + position_offset + vec3(0, heightMapVal.x * max_height, 0);"
		"   vec4 outPos = transform * vec4(pos,1);\n"
		"	gl_Position = outPos;\n"
        "	norm = mat3(model_matrix) * heightMapVal.yzw;\n"
        "	lightIntensity = max(dot(global_light_direction, norm),ambient_color.x);\n"
		"   dist = distance(vec3(0,0,0), outPos.xyz);\n"
        "}\n"
    );

    std::string Shaders::m_defaultFragmentShader = std::string(
        "#version 330 core\n"
        "\n"
		"in vec3 pos;\n"
		"in vec4 heightMapVal;\n"
		"in float lightIntensity;\n"
		"in float dist;\n"
		"\n"
		"uniform float max_height;\n"
		"uniform float fog_distance;\n"
		"\n"
        "uniform sampler2D heightmap;\n"
		"\n"
		"out vec4 color;\n"
        "\n"
        "void main()\n"
        "{\n"
		"   float val = max(lightIntensity * (pos.y / max_height), min(pow(dist, 2)/pow(fog_distance,2), 0.6f));\n"
		"   color = vec4(val,val,val, 1);\n"	
        "}\n"
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