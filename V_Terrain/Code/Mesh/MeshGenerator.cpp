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

	void MeshGenerator::MeshData::Generate(const PerlinNoise::NoiseMap & map)
	{
        m_data.resize(map.Width()*map.Height() * 8 + 1);
		m_indices.reserve((map.Width() - 1)*(map.Height() - 1) * 6 + 1);

		//Subtracting 1 if Width is odd
		float topLeftX = (map.Width() - (map.Width() % 2 != 0)) / 2.f;
		float topLeftY = (map.Height() - (map.Height() % 2 != 0)) / 2.f;
		
        //100 ms
		for (uint y = 1; y < map.Height() - 1; y++)
		{
            for (uint x = 1; x < map.Width() - 1; x++)
            {
                bool addedNorm = false;
                AddVertex(Vec3<float>(topLeftX - x, map[x + y * map.Width()] * Config::maxHeight, topLeftY - y));
                Vec2<float> a((float)(x - 1) / (float)(map.Width() - 1), (float)(y - 1) / (float)(map.Height() - 2));
                AddUV(Vec2<float>((float)(x - 1) / (float)(map.Width() - 3), (float)(y - 1) / (float)(map.Height() - 3)));

                if (x < map.Width() - 2 && y < map.Height() - 2)
                {
                    const uint index = m_nVertices - 1;
                    AddTri(index, index + map.Width() - 2, index + map.Width() + 1 - 2);
                    AddTri(index + map.Width() + 1 - 2, index + 1, index);
                }

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

    uint MeshGenerator::Mesh::m_shaderProgram = 0;

    void MeshGenerator::Mesh::Generate(const MeshData & meshData)
    {
        if (!m_used)
        {
            glGenBuffers(1, (GLuint*) &(m_dataBuff));
            glGenBuffers(1, (GLuint*) &(m_indicesBuff));
            m_used = true;
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
        if (m_used)
        {
            glDeleteBuffers(1, &m_dataBuff);
            glDeleteBuffers(1, &m_indicesBuff);
            m_used = false;
            m_nIndices = 0;
        }
    }
    void MeshGenerator::Mesh::Render(const float* viewMatrix, const float* projectionMatrix, const Vec3<int>& offset)
    {
        if (m_used)
        {
            glUseProgram(m_shaderProgram);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

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