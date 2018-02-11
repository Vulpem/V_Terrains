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
#include "MeshGenerator.h"

#include "../ExternalLibs/Glew/include/glew.h"

namespace VTerrain
{
    MeshGenerator::MeshData::MeshData()
    {
    }

    MeshGenerator::MeshData::~MeshData()
    {
    }

    void MeshGenerator::MeshData::AddVertex(const Vec3<float>& v)
    {
        m_data[m_nVertices * 8 + 0] = v.x();
        m_data[m_nVertices * 8 + 1] = v.y();
        m_data[m_nVertices * 8 + 2] = v.z();
        m_nVertices++;
    }

    void MeshGenerator::MeshData::AddUV(const Vec2<float>& uv)
    {
        m_data[m_nUVs * 8 + 6 + 0] = uv.x();
        m_data[m_nUVs * 8 + 6 + 1] = uv.y();
        m_nUVs++;
    }

    void MeshGenerator::MeshData::AddNormal(const Vec3<float>& n)
    {
        m_data[m_nNormals * 8 + 3 + 0] = n.x();
        m_data[m_nNormals * 8 + 3 + 1] = n.y();
        m_data[m_nNormals * 8 + 3 + 2] = n.z();
        m_nNormals++;
    }

	void MeshGenerator::MeshData::GenerateData(const PerlinNoise::NoiseMap & map)
	{
        m_data.resize((map.Width()-2)*(map.Height()-2) * 8);

		//Subtracting 1 if Width is odd
		float topLeftX = (map.Width() - (map.Width() % 2 != 0)) / 2.f;
		float topLeftY = (map.Height() - (map.Height() % 2 != 0)) / 2.f;

		for (uint y = 1; y < map.Height() - 1; y++)
		{
            for (uint x = 1; x < map.Width() - 1; x++)
            {
                AddVertex(Vec3<float>(topLeftX - x, map[x + y * map.Width()] * Config::maxHeight, topLeftY - y));
                AddUV(Vec2<float>((float)(x - 1) / (float)(map.Width() - 3), (float)(y - 1) / (float)(map.Height() - 3)));

                const Vec3<float> central(topLeftX - x, map[x + y * map.Width()] * Config::maxHeight, topLeftY - y);
                const Vec3<float> top =    central - Vec3<float>(topLeftX - x,     map[x + (y + 1) * map.Width()] * Config::maxHeight, topLeftY - (y + 1));
                const Vec3<float> bottom = central - Vec3<float>(topLeftX - x,     map[x + (y - 1) * map.Width()] * Config::maxHeight, topLeftY - (y - 1));
                const Vec3<float> right =  central - Vec3<float>(topLeftX - x + 1, map[x + 1 + y * map.Width()] * Config::maxHeight,   topLeftY - y);
                const Vec3<float> left =   central - Vec3<float>(topLeftX - x - 1, map[x - 1 + y * map.Width()] * Config::maxHeight,   topLeftY - y);

                Vec3<float> norm =
                    top.Cross(left)
                    + left.Cross(bottom)
                    + bottom.Cross(right)
                    + right.Cross(top);
                norm.Normalize();

                AddNormal(norm);
            }
		}
	}

    



    std::map<uint, uint> MeshGenerator::Mesh::m_indicesBuff;
    std::map<uint, uint> MeshGenerator::Mesh::m_nIndices;
    uint MeshGenerator::Mesh::m_shaderProgram = 0;

    void MeshGenerator::Mesh::Generate(const MeshData & meshData)
    {
        if (m_bufferGenerated == false)
        {
            glGenBuffers(1, (GLuint*) &(m_dataBuff));
            m_bufferGenerated = true;
        }
       
        glBindBuffer(GL_ARRAY_BUFFER, m_dataBuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * meshData.m_data.size(), meshData.m_data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void MeshGenerator::Mesh::GenerateIndices(uint width, uint height, uint LOD)
    {
        //Number of LODs is too high
        assert(pow(2, Config::nLODs-1) <= Config::chunkWidth);
        assert(pow(2, Config::nLODs-1) <= Config::chunkHeight);

        std::vector<uint> indices((width)*(height) * 6);
        uint n = 0;
        uint i = 0;
        uint step = pow(2, LOD);
        for (uint y = 0; y < height + 1; y += step)
        {
            i = (width + 1)*y;
            for (uint x = 0; x < width + 1; x += step)
            {
                const uint b = (width + 1)*(y + 1)-1;
                const uint c = (width + 1)*(y + 1);
                const uint d = (width + 1)*(y + 2) - 1;

                if (x < width && y < height)
                {
                    indices[n++] = i;
                    indices[n++] = i + (width + 1) * step;
                    indices[n++] = i + (width + 1) * step + step;

                    indices[n++] = i + (width + 1) * step + step;
                    indices[n++] = i + step;
                    indices[n++] = i;
                }
                i += step;
            }
        }

        std::vector<Vec3<uint>> tmp;
        for (int a = 0; a < n; a += 3)
        {
            tmp.push_back(Vec3<uint>(indices[a], indices[a + 1], indices[a + 2]));
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

    void MeshGenerator::Mesh::FreeMesh()
    {
        if (m_bufferGenerated)
        {
            glDeleteBuffers(1, &m_dataBuff);
            m_bufferGenerated = false;
        }
    }
    void MeshGenerator::Mesh::Render(const float* viewMatrix, const float* projectionMatrix, const Vec3<int>& offset, uint LOD)
    {
        if (m_bufferGenerated)
        {
            glUseProgram(m_shaderProgram);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);

            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Config::TMP::debugTexBuf);

            // ------ Setting uniforms -------------------------
            //Model Matrix
            float identity[] = { 1,0,0,0,
                                 0,1,0,0,
                                 0,0,1,0,
                                 0,0,0,1 };
            GLint modelLoc = glGetUniformLocation(m_shaderProgram, "model_matrix");
            if (modelLoc != -1) { glUniformMatrix4fv(modelLoc, 1, GL_FALSE, identity); }

            //View matrix
            GLint viewLoc = glGetUniformLocation(m_shaderProgram, "view_matrix");
            if (viewLoc != -1)
            { glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix); }

            //Projection Matrix
            GLint projectionLoc = glGetUniformLocation(m_shaderProgram, "projection_matrix");
            if (projectionLoc != -1)
            { glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMatrix); }

            GLint offsetLoc = glGetUniformLocation(m_shaderProgram, "position_offset");
            if (offsetLoc != -1)
            {
                float tmp[3] = { offset.x(), offset.y(), offset.z() };
                glUniform3fv(offsetLoc, 1, tmp);
            }

            float col[] ={ 0.5f, 0.5f, 0.5f, 1.0f };
            //Ambient color
            GLint ambientColorLoc = glGetUniformLocation(m_shaderProgram, "ambient_color");
            if (ambientColorLoc != -1)
            { glUniform4fv(ambientColorLoc, 1, col); }

            float materialCol[] = { 1.0f,1.0f,1.0f,1.0f };
            //Material color
            GLint materialColorLoc = glGetUniformLocation(m_shaderProgram, "material_color");
            if (materialColorLoc != -1)
            {
                glUniform4fv(materialColorLoc, 1, materialCol);
            }
            

            Vec3<float> dir( Config::globalLight[0], Config::globalLight[1], Config::globalLight[2]);
			dir.Normalize();
            //Global light direction
            GLint globalLightDirLoc = glGetUniformLocation(m_shaderProgram, "global_light_direction");
            if (globalLightDirLoc != -1)
            { glUniform3fv(globalLightDirLoc, 1, dir.Data()); }


            glBindBuffer(GL_ARRAY_BUFFER, m_dataBuff);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuff[LOD]);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, m_nIndices[LOD], GL_UNSIGNED_INT, (void*)0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glPopMatrix();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);

            glUseProgram(0);
        }
    }
}