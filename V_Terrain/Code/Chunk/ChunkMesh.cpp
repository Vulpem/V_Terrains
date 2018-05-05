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
#include "ChunkMesh.h"

#include "../ExternalLibs/Glew/include/glew.h"

namespace VTerrain
{
    Mesh::Mesh()
        : m_bufferGenerated(false)
        , m_generated(false)
        , m_indicesBuff(0)
        , m_nIndices(0)
        , m_dataBuff(0)
    {
    }
    Mesh::~Mesh()
    {
        Free();
    }

    void Mesh::Generate()
    {
        GenerateMesh();
        GenerateIndices();
        m_generated = true;
    }

    void Mesh::GenerateMesh()
    {
        const unsigned int width = config.chunkMinDensity + 1;
        const unsigned int height = config.chunkMinDensity + 1;

        std::vector<float> data;
        data.resize(width * height * 5);

        const float topLeft = -0.5f;
        uint n = 0;

        for (uint y = 0; y < height; y++)
        {
            for (uint x = 0; x < width; x++)
            {
                data[n++] = (float)x / (float)config.chunkMinDensity - 0.5f;
                data[n++] = 0.f;
                data[n++] = (float)y / (float)config.chunkMinDensity - 0.5f;
                data[n++] = (float)x / (float)config.chunkMinDensity;
                data[n++] = (float)y / (float)config.chunkMinDensity;
            }
        }


        if (m_dataBuff == 0)
        {
            glGenBuffers(1, (GLuint*) &(m_dataBuff));
        }
       
        glBindBuffer(GL_ARRAY_BUFFER, m_dataBuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Mesh::GenerateIndices()
    {
        const uint width = config.chunkMinDensity + 1;
        const uint height = config.chunkMinDensity + 1;

        std::vector<uint> indices;
        indices.reserve((width - 1)*(height - 1) * 4u);

        uint n = 0;
        uint i = 0;
        const uint step = 1;
        for (uint y = 0; y < height; y += step)
        {
            i = (width)*y;
            for (uint x = 0; x < width; x += step)
            {
                if (x < width - 1 && y < height - 1)
                {
                    indices.push_back(i);
                    indices.push_back(i + (width)* utils::Min(step, height - y));
                    indices.push_back(i + (width)* utils::Min(step, height - y) + utils::Min(step, width - x));
                    indices.push_back(i + utils::Min(step, width - x));
                }
                i += step;
            }
        }
        m_nIndices = indices.size();

        if (m_indicesBuff == 0)
        {
            glGenBuffers(1, &m_indicesBuff);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuff);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * m_nIndices, indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void Mesh::FreeMesh()
    {
        if (m_dataBuff != 0)
        {
            glDeleteBuffers(1, &m_dataBuff);
            m_dataBuff = 0;
        }
    }

    void Mesh::FreeIndices()
    {
        if (m_indicesBuff != 0)
        {
            glDeleteBuffers(1, &m_indicesBuff);
            m_indicesBuff = 0;
        }
    }

    void Mesh::Free()
    {
        if (m_generated)
        {
            FreeMesh();
            FreeIndices();
            m_generated = false;
        }
    }
}