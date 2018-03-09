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
        : m_generated(false)
    {
    }
    Mesh::~Mesh()
    {
        Free();
    }

    void Mesh::Generate()
    {
        GenerateMesh();
        for (uint lod = 0; lod < config.nLODs; lod++)
        {
            GenerateIndices(lod);
        }
        m_generated = true;
    }

    void Mesh::GenerateMesh()
    {
        std::vector<float> data;
        data.resize((config.chunkWidth + 1) * (config.chunkHeight + 1) * 5);

        const float topLeftX = (config.chunkWidth * config.quadSize - (config.chunkWidth % 2 != 0)) / -2.f;
        const float topLeftY = (config.chunkHeight * config.quadSize - (config.chunkHeight % 2 != 0)) / -2.f;
        uint n = 0;

        std::vector<Vec2<float>> UVs;

        for (uint y = 0; y < config.chunkHeight + 1; y++)
        {
            for (uint x = 0; x < config.chunkWidth + 1; x++)
            {
                data[n + 0] = topLeftX + x * config.quadSize;
                data[n + 1] = 0.f;
                data[n + 2] = topLeftY + y * config.quadSize;
                data[n + 3] = (float)(x) / (float)(config.chunkWidth);
                data[n + 4] = (float)(y) / (float)(config.chunkHeight);
                n += 5;
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

    void Mesh::GenerateIndices(uint LOD)
    {
        const uint width = config.chunkWidth + 1;
        const uint height = config.chunkHeight + 1;
        std::vector<uint> indices(width*height * 6u);
        uint n = 0;
        uint i = 0;
        uint step = static_cast<uint>(powf(2, static_cast<float>(LOD)));
        for (uint y = 0; y < height; y += step)
        {
            i = (width)*y;
            for (uint x = 0; x < width; x += step)
            {
                if (x < width - 1 && y < height - 1)
                {
                    indices[n++] = i;
                    indices[n++] = i + (width) * utils::Min(step, height - y);
                    indices[n++] = i + (width) * utils::Min(step, height - y) + utils::Min(step, width - x);

                    indices[n++] = i + (width) * utils::Min(step, height - y) + utils::Min(step, width - x);
                    indices[n++] = i + utils::Min(step, width - x);
                    indices[n++] = i;
                }
                i += step;
            }
        }

        m_nIndices[LOD] = n;
        if (m_indicesBuff.find(LOD) == m_indicesBuff.end())
        {
            glGenBuffers(1, &m_indicesBuff[LOD]);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuff[LOD]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * m_nIndices[LOD], indices.data(), GL_STATIC_DRAW);
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

    void Mesh::FreeLOD(uint lod)
    {
        auto it = m_indicesBuff.find(lod);
        if ( it != m_indicesBuff.end())
        {
            glDeleteBuffers(1, &it->second);
            m_nIndices.erase(it->first);
            m_indicesBuff.erase(it);
        }
    }

    void Mesh::Free()
    {
        if (m_generated)
        {
            FreeMesh();
            while (m_indicesBuff.size() > 0)
            {
                FreeLOD(m_nIndices.begin()->first);
            }
            m_generated = false;
        }
    }
}