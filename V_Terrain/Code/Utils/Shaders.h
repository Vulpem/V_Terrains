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
#pragma once

#include <string>

namespace VTerrain
{
    struct Shader
    {
        unsigned int m_program = 0u;

        unsigned int attrib_vertex = 0u;
        unsigned int attrib_UV = 0u;

        unsigned int loc_position = 0u;
        unsigned int loc_texCoord = 0u;

        unsigned int loc_view_matrix = 0u;
        unsigned int loc_projection_matrix = 0u;

        unsigned int loc_position_offset = 0u;
        unsigned int loc_max_height = 0u;
        unsigned int loc_fog_distance = 0u;
        unsigned int loc_water_height = 0u;
        unsigned int loc_global_light_direction = 0u;
        unsigned int loc_ambient_color = 0u;
        unsigned int loc_water_color = 0u;
        unsigned int loc_fog_color = 0u;

        unsigned int loc_render_heightmap = 0u;
        unsigned int loc_render_chunk_borders = 0u;
		unsigned int loc_render_light = 0u;
    };

    class Shaders
    {
    public:
        static Shader CompileShader(const char* vertexBuf, const char* fragmentBuf, const char* TCS, const char* TES, std::string& result);
        static void FreeShader(const Shader& shader);

        static std::string m_defaultVertexShader;
        static std::string m_defaultFragmentShader;
        static std::string m_defaultTCSShader;
        static std::string m_defaultTESShader;
    private:
        static std::string OpenFile(const char* fileDir);
        static unsigned int Compile(std::string code, unsigned int type, std::string& result);
    };
}