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
//  You should have received a copy of the GNU General Public License along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
#include "Shaders.h"

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
		"out float lightIntensity;\n"
        "out vec2 TexCoord;\n"
        "\n"
        "uniform mat4 model_matrix;\n"
        "uniform mat4 view_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "uniform vec3 position_offset;\n"
        "uniform vec3 global_light_direction;\n"
        "uniform vec4 ambient_color;\n"
        "uniform float max_height;\n"
        "\n"
        "uniform sampler2D heightmap;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	mat4 transform = projection_matrix * view_matrix * model_matrix;\n"
        "   vec4 height = texture(heightmap, texCoord);\n"
        "	gl_Position = transform * vec4(position + position_offset, 1.0f) + vec4(0, height.x * max_height, 0, 0);\n"
        "		vec3 norm = mat3(model_matrix) * height.yzw;\n"
        "		lightIntensity = dot(global_light_direction, norm);\n"
        "		lightIntensity = max(lightIntensity,ambient_color.x);\n"
		"       pos = position + position_offset;\n"
        "	TexCoord = texCoord;\n"
        "}\n"
    );

    std::string Shaders::m_defaultFragmentShader = std::string(
        "#version 330 core\n"
        "\n"
		"in vec3 pos;\n"
		"in float lightIntensity;\n"
        "in vec2 TexCoord;\n"
        "\n"
        "out vec4 color;\n"
        "\n"
        "uniform sampler2D heightmap;\n"
        "\n"
        "void main()\n"
        "{\n"
		"       float val = (ceil(pos.y / 10)/10) * lightIntensity;\n"
		"       vec4 mat = vec4(val,val,val, 1);\n"
        "       color = /*mat * */texture(heightmap, TexCoord);\n"
        "}\n"
    );

    std::string Shaders::CompileShader(const char * vertexBuf, const char * fragmentBuf, uint & shaderProgram)
    {
        std::string ret;
        bool error = false;
        shaderProgram = 0;
        GLint success;

        ret += "\n------ Vertex shader ------\n";
        GLuint vertexShader;
        if (vertexBuf == nullptr)
        {
            ret += "- No vertex shader found. Using default vertex shader.\n";
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
        }
        else
        {
            ret += "Compilation succesfull\n";
        }

        ret += "\n------ Fragment shader ------\n";
        GLuint fragmentShader;
        if (fragmentBuf == nullptr)
        {
            ret += "- No fragment shader found. Using default fragment shader.\n";
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
        }
        else
        {
            ret += "Compilation succesfull\n";
        }

        GLuint program;
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

    void Shaders::FreeShader(uint shaderProgram)
    {
        glDeleteProgram(shaderProgram);
    }
}