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
#include <windows.h> 

namespace VTerrain
{
    std::string Shaders::m_defaultVertexShader = std::string("Here will go the shader TODO");

	std::string Shaders::m_defaultFragmentShader = std::string("Here will go the shader TODO");

    std::string Shaders::m_defaultTCSShader = std::string("TODO");
    std::string Shaders::m_defaultTESShader = std::string("TODO");

    Shader Shaders::CompileShader(const char * vertexBuf, const char * fragmentBuf, const char* TCSbuf, const char* TESbuf, std::string& result)
    {
        GLint success;
        Shader ret;

        //TODO customizable
        unsigned int vertexShader = 0;
        if (vertexBuf == nullptr)
        {
            vertexShader = Compile(OpenFile("vertex.cpp"), GL_VERTEX_SHADER, result);
        }
        else
        {
            vertexShader = Compile(vertexBuf, GL_VERTEX_SHADER, result);
        }
        unsigned int fragmentShader = 0;
        if (fragmentBuf == nullptr)
        {
            fragmentShader = Compile(OpenFile("fragment.cpp"), GL_FRAGMENT_SHADER, result);
        }
        else
        {
            fragmentShader = Compile(fragmentBuf, GL_FRAGMENT_SHADER, result);
        }

        unsigned int TCS = 0;
        if (TCSbuf == nullptr)
        {
            TCS = Compile(OpenFile("TCS.cpp"), GL_TESS_CONTROL_SHADER, result);
        }
        else
        {
            TCS = Compile(TCSbuf, GL_TESS_CONTROL_SHADER, result);
        }

        unsigned int TES = 0;
        if (TCSbuf == nullptr)
        {
            TES = Compile(OpenFile("TES.cpp"), GL_TESS_EVALUATION_SHADER, result);
        }
        else
        {
            TES = Compile(TESbuf, GL_TESS_EVALUATION_SHADER, result);
        }

        if (fragmentShader != 0 && vertexShader != 0 && TCS != 0 && TES != 0)
        {
            unsigned int program;
            program = glCreateProgram();

            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glAttachShader(program, TCS);
            glAttachShader(program, TES);

            glLinkProgram(program);
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (success == 0)
            {
                GLchar infoLog[512];
                glGetProgramInfoLog(program, 512, NULL, infoLog);
                result += "\n------ Shader Program ------\n";
                result += infoLog;
                result += '\n';
            }
            else
            {
                ret.m_program = program;

                ret.attrib_vertex = glGetAttribLocation(program, "position");
                ret.attrib_UV = glGetAttribLocation(program, "texCoord");

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

                ret.loc_maxLOD = glGetUniformLocation(program, "maxDensity");
                ret.loc_tesselationDensity = glGetUniformLocation(program, "tesselationDensity");

                ret.loc_render_chunk_borders = glGetUniformLocation(program, "render_chunk_borders");
                ret.loc_render_heightmap = glGetUniformLocation(program, "render_heightmap");
				ret.loc_render_light = glGetUniformLocation(program, "render_light");

				for (int n = 0; n < 10; n++)
				{
					char val[28];
					sprintf_s(val, 28, "textures[%i].", n);
					ret.textures[n].loc_diffuse = glGetUniformLocation(program, (std::string(val) + "diffuse").data());
					ret.textures[n].loc_heightmap = glGetUniformLocation(program, (std::string(val) + "heightmap").data());

					ret.textures[n].data = glGetUniformLocation(program, (std::string(val) + "data").data());
				}
            }

			ret.loc_heightmap = glGetUniformLocation(program, "heightmap");

            glDetachShader(program, vertexShader);
            glDetachShader(program, fragmentShader);

        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return ret;
    }

    void Shaders::FreeShader(const Shader& shader)
    {
        glDeleteProgram(shader.m_program);
    }
    std::string Shaders::OpenFile(const char * fileDir)
    {
        TCHAR pwd[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, pwd);

        std::string dir(pwd);
        dir += "/../V_Terrain/Code/Shaders/";
        dir += fileDir;
        std::string ret;
        std::ifstream inStream;
        inStream.open(dir.data());
        if (inStream.is_open())
        {
            while (!inStream.eof())
            {
                std::string tmp;
                std::getline(inStream, tmp);
                ret += tmp;
                ret += '\n';
            }
            inStream.close();
        }
        return ret;
    }

    unsigned int Shaders::Compile(std::string code, unsigned int type, std::string& result)
    {
        unsigned int ret;
        GLint success;

        ret = glCreateShader(type);
        const char* tmp = code.c_str();
        glShaderSource(ret, 1, &tmp, NULL);
        glCompileShader(ret);
        glGetShaderiv(ret, GL_COMPILE_STATUS, &success);
        if (success == 0)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(ret, 512, NULL, infoLog);
            result += infoLog;
            result += '\n';
            result += '\n';
            glDeleteShader(ret);
            ret = 0;
        }

        switch (type)
        {
        case GL_VERTEX_SHADER:
            m_defaultVertexShader = code;
            break;
        case GL_FRAGMENT_SHADER:
            m_defaultFragmentShader = code;
            break;
        case GL_TESS_CONTROL_SHADER:
            m_defaultTCSShader = code;
            break;
        case GL_TESS_EVALUATION_SHADER:
            m_defaultTESShader = code;
            break;
        }

        return ret;
    }
}