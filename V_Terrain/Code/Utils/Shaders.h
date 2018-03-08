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
        std::string m_frag_result;
        std::string m_vert_result;
        std::string m_program_result;

        unsigned int m_program;

        unsigned int loc_position;
        unsigned int loc_texCoord;

        unsigned int loc_model_matrix;
        unsigned int loc_view_matrix;
        unsigned int loc_projection_matrix;

        unsigned int loc_position_offset;
        unsigned int loc_max_height;
        unsigned int loc_fog_distance;
        unsigned int loc_water_height;
        unsigned int loc_global_light_direction;
        unsigned int loc_ambient_color;
        unsigned int loc_water_color;
        unsigned int loc_fog_color;
    };

    class Shaders
    {
    public:
        static Shader CompileShader(const char* vertexBuf, const char* fragmentBuf);
        static void FreeShader(const Shader& shader);

        static std::string m_defaultVertexShader;
        static std::string m_defaultFragmentShader;
    };
}