#ifndef  __R_MESH__
#define __R_MESH__

#include "R_Resource.h"

#include "OpenGL.h"

class R_Mesh : public Resource
{
public:
	R_Mesh() :Resource() { aabb.SetNegativeInfinity(); }
	R_Mesh(uint64_t UID) : Resource(UID) { aabb.SetNegativeInfinity(); }

	~R_Mesh()
	{
		if (id_indices != 0)
		{
			glDeleteBuffers(1, &id_indices);
		}
		if (id_data != 0)
		{
			glDeleteBuffers(1, &id_data);
		}		

		RELEASE_ARRAY(vertices);
		RELEASE_ARRAY(indices);
		RELEASE_ARRAY(normals);
	}

	Component::Type GetType() { return Component::Type::C_mesh; }

	uint* indices = nullptr;
	uint id_indices = 0;
	uint m_nIndices = 0;

	uint m_nVertices = 0;
	float3* vertices = nullptr;	
	float3* normals = nullptr;

	uint id_data = 0;

	bool hasNormals = false;
	bool hasUVs = false;

	int defaultMaterialIndex = -1;

	AABB aabb;
};

#endif