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
        GenerateIndices();
        m_generated = true;
    }

    void Mesh::GenerateMesh()
    {
        std::vector<float> data;
        data.resize(20);

		//Vert 0
		data[0] = -0.5f;
		data[1] = 0.f;
		data[2] = -0.5f;
		data[3] = 0.f;
		data[4] = 0.f;

		//Vert 1
		data[5] = 0.5f;
		data[6] = 0.f;
		data[7] = -0.5f;
		data[8] = 1.f;
		data[9] = 0.f;

		//Vert 2
		data[10] = -0.5f;
		data[11] = 0.f;
		data[12] = 0.5f;
		data[13] = 0.f;
		data[14] = 1.f;

		//Vert 3
		data[15] = 0.5f;
		data[16] = 0.f;
		data[17] = 0.5f;
		data[18] = 1.f;
		data[19] = 1.f;

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
		std::vector<uint> indices;
		indices.resize(4);

		indices[0] = 0;
		indices[1] = 2;
		indices[2] = 3;
		indices[3] = 1;
        
        if (m_indicesBuff == 0)
        {
            glGenBuffers(1, &m_indicesBuff);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuff);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), indices.data(), GL_STATIC_DRAW);
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