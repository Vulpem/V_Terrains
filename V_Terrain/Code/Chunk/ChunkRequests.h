//  RPG Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hern�ndez L�zaro
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

#include "../Globals.h"

namespace RPGT
{
    struct RequestedChunk
    {
        RequestedChunk(int x = 0, int y = 0) : pos(x, y) {}
        RequestedChunk(Vec2<int> p) : pos(p) {}
        Vec2<int> pos;
        bool operator== (const RequestedChunk& p) { return pos == p.pos; }
        bool operator== (const Vec2<int>& p) { return pos == p; }
    };

    struct GeneratedChunk
    {
        GeneratedChunk() = default;
        Vec2<int> m_pos;
        Vec2<uint> m_size;
        std::vector<float> m_data;
        float& operator[] (int n) { return m_data[n]; }
    };
}