#ifndef __MESH__
#define __MESH__

#include "ResourcedComponent.h"

struct Mesh_RenderInfo;

class Mesh : public  ResourcedComponent
{
public:
	Mesh(std::string resource, Gameobject* linkedTo);

	Mesh_RenderInfo GetMeshInfo() const;

	const float3* GetVertices() const;
	const uint GetNumVertices();
	const uint* GetIndices() const;
	const uint GetNumIndices();
	const float3* GetNormals() const;

	AABB GetAABB();

	void EditorContent() override;

	void SaveSpecifics(pugi::xml_node& myNode) override;

	void LoadSpecifics(pugi::xml_node & myNode) override;

	ComponentType GetType() const override { return ComponentType::mesh; }

	int m_textureIndex = -1;

	bool m_drawWired = false;
	bool m_drawNormals = false;
};

#endif