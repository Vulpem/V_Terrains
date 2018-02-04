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

    void MeshGenerator::MeshData::AddVertex(double x, double y, double z)
    {
        m_vertices.push_back(x);
        m_vertices.push_back(y);
        m_vertices.push_back(z);
    }

    void MeshGenerator::MeshData::AddUV(double x, double y)
    {
        m_UVs.push_back(x);
        m_UVs.push_back(y);
    }

    void MeshGenerator::MeshData::AddNormal(double x, double y, double z)
    {
        m_normals.push_back(x);
        m_normals.push_back(y);
        m_normals.push_back(z);
    }

    void MeshGenerator::MeshData::Generate(const PerlinNoise::NoiseMap & map)
    {
        m_vertices.reserve(map.Width()*map.Height() * 3);
        m_UVs.reserve(map.Width()*map.Height() * 2);
        m_normals.reserve(map.Width()*map.Height() * 3);
        m_indices.reserve((map.Width() - 1)*(map.Height() - 1) * 6);

        float topLeftX = (map.Width() - 1) / 2.f;
        float topLeftY = (map.Height() - 1) / 2.f;

        for (uint y = 0; y < map.Height(); y++)
        {
            for (uint x = 0; x < map.Width(); x++)
            {
                AddVertex(topLeftX + x, map[x + y * map.Width()], topLeftY - y);
                AddUV(x / static_cast<float>(map.Width()), y / static_cast<float>(map.Width()));

                if (x < map.Width() - 1 && y < map.Height() - 1)
                {
                    uint index = m_indices.size() - 1;
                    AddTri(index, index + map.Width() + 1, index + map.Width());
                    AddTri(index + map.Width() + 1, index, index + 1);
                }
            }
        }

        CompressData();
    }
    void MeshGenerator::MeshData::CompressData()
    {
        m_data.reserve(m_vertices.size() * (3 + 3 + 2));

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
                m_data.push_back(0.);
                m_data.push_back(0.);
                m_data.push_back(0.);
            }

            if (n < m_UVs.size())
            {
                m_data.push_back(m_UVs[n * 2 + 0]);
                m_data.push_back(m_UVs[n * 2 + 1]);
            }
            else
            {
                m_data.push_back(0.);
                m_data.push_back(0.);
            }
        }
    }

    void MeshGenerator::Mesh::Generate(const MeshData & meshData)
    {
        if (used)
        {
            FreeMesh();
        }
        used = true;
        //Generating data buffer
        glGenBuffers(1, (GLuint*) &(m_dataBuff));
        glBindBuffer(GL_ARRAY_BUFFER, m_dataBuff);
        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * meshData.m_data.size(), meshData.m_data.data(), GL_STATIC_DRAW);

        glGenBuffers(1, (GLuint*) &(m_indicesBuff));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuff);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * meshData.m_indices.size(), meshData.m_indices.data(), GL_STATIC_DRAW);

    }

    void MeshGenerator::Mesh::FreeMesh()
    {
        if (used)
        {
            glDeleteBuffers(1, &m_dataBuff);
            glDeleteBuffers(1, &m_indicesBuff);
            used = false;
        }
    }
    void MeshGenerator::Mesh::Render()
    {
    }
}