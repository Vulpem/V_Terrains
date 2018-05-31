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
    Shader Shaders::CompileShader(const char * vertexBuf, const char * fragmentBuf, const char* TCSbuf, const char* TESbuf, std::string& result)
    {
        //Checking OpenGL version
        std::string fullVersion = (char*)glGetString(GL_VERSION);
        std::string simpleVersion = fullVersion.substr(0, fullVersion.find_first_of("."));
        const int version = std::stoi(simpleVersion);
        //It must be 4.x or higher in order to support tesselation
        ASSERT(version >= 4, "OpenGL version is not compatible. \nCurrent version is %s.\nMinimum version required is 4.0.0", fullVersion.data());

        GLint success;
        Shader ret;
        std::string code;

        unsigned int vertexShader = 0;
        if (vertexBuf == nullptr)
        {
            code = OpenFile("vertex.cpp");
            if(code.length() <= 1)
            {
                code = GetDefaultVertexShader();
            }
            vertexShader = Compile(code, GL_VERTEX_SHADER, result);
        }
        else
        {
            vertexShader = Compile(vertexBuf, GL_VERTEX_SHADER, result);
        }

        unsigned int TCS = 0;
        if (TCSbuf == nullptr)
        {
            code = OpenFile("TCS.cpp");
            if (code.length() <= 1)
            {
                code = GetDefaultTCSShader();
            }
            TCS = Compile(code, GL_TESS_CONTROL_SHADER, result);
        }
        else
        {
            TCS = Compile(TCSbuf, GL_TESS_CONTROL_SHADER, result);
        }

        unsigned int TES = 0;
        if (TCSbuf == nullptr)
        {
            code = OpenFile("TES.cpp");
            if (code.length() <= 1)
            {
                code = GetDefaultTESShader();
            }
            TES = Compile(code, GL_TESS_EVALUATION_SHADER, result);
        }
        else
        {
            TES = Compile(TESbuf, GL_TESS_EVALUATION_SHADER, result);
        }

        unsigned int fragmentShader = 0;
        if (fragmentBuf == nullptr)
        {
            code = OpenFile("fragment.cpp");
            if (code.length() <= 1)
            {
                code = GetDefaultFragmentShader();
            }
            fragmentShader = Compile(code, GL_FRAGMENT_SHADER, result);
        }
        else
        {
            fragmentShader = Compile(fragmentBuf, GL_FRAGMENT_SHADER, result);
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
                glDeleteProgram(program);
            }
            else
            {
                ret.m_program = program;

                ret.attrib_vertex = glGetAttribLocation(program, "position");
                ret.attrib_UV = glGetAttribLocation(program, "texCoord");

                ret.loc_view_matrix = glGetUniformLocation(program, "view_matrix");
                ret.loc_projection_matrix = glGetUniformLocation(program, "projection_matrix");
                ret.loc_model_matrix = glGetUniformLocation(program, "model_matrix");
                ;
                ret.loc_global_light_direction = glGetUniformLocation(program, "global_light_direction");
                ret.loc_fog_distance = glGetUniformLocation(program, "fog_distance");
                ret.loc_fog_color = glGetUniformLocation(program, "fog_color");
                ret.loc_water_height = glGetUniformLocation(program, "water_height");
                ret.loc_ambient_color = glGetUniformLocation(program, "ambient_min");

                ret.loc_maxLOD = glGetUniformLocation(program, "maxDensity");

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
            glDetachShader(program, TCS);
            glDetachShader(program, TES);

        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(TCS);
        glDeleteShader(TES);

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

    void Shaders::SaveFile(const std::string& file, const char * fileDir)
    {
        TCHAR pwd[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, pwd);

        std::string dir(pwd);
        dir += "/../V_Terrain/Code/Shaders/";
        dir += fileDir;
        std::string ret;
        std::ofstream outStream;
        outStream.open(dir.data());
       
        if (outStream.is_open())
        {
            outStream.write(file.data(), file.size());
            outStream.close();
        }
    }

    const char* Shaders::GetShaderType(unsigned int type)
    {
        switch (type)
        {
        case GL_VERTEX_SHADER:
            return "Vertex Shader";
        case GL_FRAGMENT_SHADER:
            return "Fragment Shader";
        case GL_TESS_CONTROL_SHADER:
            return "TCS Shader";
        case GL_TESS_EVALUATION_SHADER:
            return "TES Shader";
        }
        return "Unspecified";
    }

    unsigned int Shaders::Compile(const std::string& code, unsigned int type, std::string& result)
    {
        unsigned int ret;
        GLint success = 0;

        ret = glCreateShader(type);

        if (ret == 0)
        {
            GLenum err = glGetError();
            while (err != 0)
            {
                const GLubyte* errorString = glewGetErrorString(err);
                result += "\n";
                result += (char*)errorString;
                err = glGetError();
            }
            ASSERT(ret != 0, "Error while compiling %s:\n%s", GetShaderType(type), result.data());
        }

        const char* tmp = code.c_str();
        glShaderSource(ret, 1, &tmp, NULL);
        glCompileShader(ret);
        glGetShaderiv(ret, GL_COMPILE_STATUS, &success);
        if (success == 0)
        {
            result += "-- ";
            result += GetShaderType(type);
            result += " -- \n";
            GLchar infoLog[512];
            glGetShaderInfoLog(ret, 512, NULL, infoLog);
            result += infoLog;
            result += '\n';
            result += '\n';
            glDeleteShader(ret);
            ret = 0;
        }

        return ret;
    }
}