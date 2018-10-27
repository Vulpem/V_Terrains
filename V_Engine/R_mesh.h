#ifndef  __R_MESH__
#define __R_MESH__

#include "R_Resource.h"

#include "OpenGL.h"

class R_Mesh : public Resource
{
public:
	R_Mesh() :Resource() { m_aabb.SetNegativeInfinity(); }
	R_Mesh(uint64_t UID) : Resource(UID) { m_aabb.SetNegativeInfinity(); }

	~R_Mesh()
	{
		if (m_idIndices != 0)
		{
			glDeleteBuffers(1, &m_idIndices);
		}
		if (m_idData != 0)
		{
			glDeleteBuffers(1, &m_idData);
		}		

		RELEASE_ARRAY(m_vertices);
		RELEASE_ARRAY(m_indices);
		RELEASE_ARRAY(m_normals);
	}

	ComponentType GetType() const override { return ComponentType::mesh; }

	uint* m_indices = nullptr;
	uint m_idIndices = 0;
	uint m_numIndices = 0;

	uint m_numVertices = 0;
	float3* m_vertices = nullptr;	
	float3* m_normals = nullptr;

	uint m_idData = 0;

	bool m_hasNormals = false;
	bool m_hasUVs = false;

	int m_defaultMaterialIndex = -1;

	AABB m_aabb;
};

#endif