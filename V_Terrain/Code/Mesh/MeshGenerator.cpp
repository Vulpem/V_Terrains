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

    void MeshGenerator::MeshData::AddTri(uint a, uint b, uint c)
    {
        m_indices.push_back(a);
        m_indices.push_back(b);
        m_indices.push_back(c);
    }

    void MeshGenerator::MeshData::AddVertex(float x, float y, float z)
    {
        m_vertices.push_back(x);
        m_vertices.push_back(y);
        m_vertices.push_back(z);
    }

    void MeshGenerator::MeshData::AddUV(float x, float y)
    {
        m_UVs.push_back(x);
        m_UVs.push_back(y);
    }

    void MeshGenerator::MeshData::AddNormal(float x, float y, float z)
    {
        m_normals.push_back(x);
        m_normals.push_back(y);
        m_normals.push_back(z);
    }

    void MeshGenerator::MeshData::Generate(const PerlinNoise::NoiseMap & map)
    {
        m_vertices.clear();
        m_UVs.clear();
        m_normals.clear();
        m_indices.clear();

        m_vertices.reserve(map.Width()*map.Height() * 3 + 1);
        m_UVs.reserve(map.Width()*map.Height() * 2 + 1);
        m_normals.reserve(map.Width()*map.Height() * 3 + 1);
        m_indices.reserve((map.Width() - 1)*(map.Height() - 1) * 6 + 1);

        //Subtracting 1 if Width is odd
        float topLeftX = (map.Width() - (map.Width() % 2 != 0) ) / -2.f;
        float topLeftY = (map.Height() - (map.Height() % 2 != 0)) / 2.f;

        for (uint y = 0; y < map.Height(); y++)
        {
            for (uint x = 0; x < map.Width(); x++)
            {
                AddVertex(topLeftX + x, map[x + y * map.Width()], topLeftY - y);
                AddUV(x / static_cast<float>(map.Width()-1), y / static_cast<float>(map.Height()-1));

                if (x < map.Width() - 1 && y < map.Height() - 1)
                {
                    uint index = (m_vertices.size()/3)-1;
                    AddTri(index, index + map.Width() + 1, index + map.Width());
                    AddTri(index + map.Width() + 1, index, index + 1);
                }
            }
        }

        CompressData();
    }
    void MeshGenerator::MeshData::CompressData()
    {
        m_data.clear();
        m_data.reserve(m_vertices.size() * (3 + 3 + 2) + 1);

        for (uint n = 0; n < m_vertices.size() / 3; n++)
        {
            m_data.push_back(m_vertices[n * 3 + 0]);
            m_data.push_back(m_vertices[n * 3 + 1]);
            m_data.push_back(m_vertices[n * 3 + 2]);

            if (n < m_normals.size())
            {
                m_data.push_back(m_normals[n * 3 + 0]);
                m_data.push_back(m_normals[n * 3 + 1]);
                m_data.push_back(m_normals[n * 3 + 2]);
            }
            else
            {
                m_data.push_back(0.f);
                m_data.push_back(1.f);
                m_data.push_back(0.f);
            }

            if (n < m_UVs.size())
            {
                m_data.push_back(m_UVs[n * 2 + 0]);
                m_data.push_back(m_UVs[n * 2 + 1]);
            }
            else
            {
                m_data.push_back(0.f);
                m_data.push_back(0.f);
            }
        }
    }

    uint MeshGenerator::Mesh::m_shaderProgram = 0;

    void MeshGenerator::Mesh::Generate(const MeshData & meshData)
    {
        if (!used)
        {
            glGenBuffers(1, (GLuint*) &(m_dataBuff));
            glGenBuffers(1, (GLuint*) &(m_indicesBuff));
            used = true;
        }
        
        m_nIndices = meshData.m_indices.size();
       
        glBindBuffer(GL_ARRAY_BUFFER, m_dataBuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * meshData.m_data.size(), meshData.m_data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuff);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * m_nIndices, meshData.m_indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void MeshGenerator::Mesh::FreeMesh()
    {
        if (used)
        {
            glDeleteBuffers(1, &m_dataBuff);
            glDeleteBuffers(1, &m_indicesBuff);
            used = false;
            m_nIndices = 0;
        }
    }
    void MeshGenerator::Mesh::Render(const float* viewMatrix, const float* projectionMatrix)
    {
        if (used)
        {
            glUseProgram(m_shaderProgram);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

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

            float col[] ={ 0.5f, 0.5f, 0.5f };
            //Ambient color
            GLint ambientColorLoc = glGetUniformLocation(m_shaderProgram, "ambient_color");
            if (ambientColorLoc != -1)
            { glUniform4fv(ambientColorLoc, 1, col); }

            float dir[] = { 0.2f,0.2f,0.2f };
            //Global light direction
            GLint globalLightDirLoc = glGetUniformLocation(m_shaderProgram, "global_light_direction");
            if (globalLightDirLoc != -1)
            { glUniform3fv(globalLightDirLoc, 1, dir); }


            glBindBuffer(GL_ARRAY_BUFFER, m_dataBuff);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuff);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, m_nIndices, GL_UNSIGNED_INT, (void*)0);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR)
            {
                char* err = (char*)glewGetErrorString(error);
            }

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