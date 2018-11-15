#ifndef __MESH__
#define __MESH__

#include "ResourcedComponent.h"

struct Mesh_RenderInfo;

class Mesh : public  ResourcedComponent
{
public:
	Mesh(std::string resource, GameObject* linkedTo);

public:
	int m_textureIndex = -1;

	bool m_drawWired = false;
	bool m_drawNormals = false;

	Mesh_RenderInfo GetMeshInfo();

	const float3* GetVertices() const;
	const uint GetNumVertices();
	const uint* GetIndices() const;
	const uint GetNumIndices();
	const float3* GetNormals() const;

	AABB GetAABB();

	void EditorContent();

	void SaveSpecifics(pugi::xml_node& myNode);

	void LoadSpecifics(pugi::xml_node & myNode);

	ComponentType GetType() const override { return ComponentType::mesh; }
};

#endif