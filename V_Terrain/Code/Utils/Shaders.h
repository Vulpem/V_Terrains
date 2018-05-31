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
#pragma once

#include <string>

namespace RPGT
{
    struct Shader
    {
        unsigned int m_program = 0u;

        unsigned int attrib_vertex = 0u;
        unsigned int attrib_UV = 0u;

        unsigned int loc_view_matrix = 0u;
        unsigned int loc_projection_matrix = 0u;
		unsigned int loc_model_matrix = 0u;

        unsigned int loc_fog_distance = 0u;
        unsigned int loc_water_height = 0u;
        unsigned int loc_global_light_direction = 0u;
        unsigned int loc_ambient_color = 0u;
        unsigned int loc_fog_color = 0u;

        unsigned int loc_maxLOD = 0u;

        unsigned int loc_render_heightmap = 0u;
        unsigned int loc_render_chunk_borders = 0u;
		unsigned int loc_render_light = 0u;

		struct textureLocs
		{
			unsigned int loc_diffuse = 0u;
			unsigned int loc_heightmap = 0u;
			unsigned int data = 0u;
		}textures[10];

		unsigned int loc_heightmap = 0u;
    };

    class Shaders
    {
    public:
        static Shader CompileShader(const char* vertexBuf, const char* fragmentBuf, const char* TCS, const char* TES, std::string& result);
        static void FreeShader(const Shader& shader);


        static std::string GetDefaultVertexShader();
        static std::string GetDefaultFragmentShader();
        static std::string GetDefaultTCSShader();
        static std::string GetDefaultTESShader();

        //TODO to remove
        static void SaveFile(const std::string& file, const char* fileDir);
    private:
        static std::string OpenFile(const char* fileDir);
		static const char* GetShaderType(unsigned int type);
        static unsigned int Compile(const std::string& code, unsigned int type, std::string& result);
    };
}