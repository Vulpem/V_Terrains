#include "Mesh.h"

#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Mesh_RenderInfo.h"

#include "Application.h"

#include "R_Mesh.h"

//------------------------- MESH --------------------------------------------------------------------------------

mesh::mesh(std::string resource, GameObject* linkedTo): ResourcedComponent(resource, linkedTo, C_mesh)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Mesh##%i", uid);
	name = tmp;

	texMaterialIndex = object->AmountOfComponent(Component::Type::C_mesh);
}

Mesh_RenderInfo mesh::GetMeshInfo()
{
	Mesh_RenderInfo ret;
	if (object->IsActive())
	{
		if (wires == true || object->selected)
		{
			ret.m_drawWired = true;
			if (wires == true)
			{
				ret.m_drawDoubleSidedFaces = true;
				ret.m_wiresColor = float4(0.f, 0.f, 0.f, 1.0f);
			}
			if (object->selected)
			{
				if (object->parent && object->parent->selected)
				{
					ret.m_wiresColor = float4(0, 0.5f, 0.5f, 1);
				}
				else
				{
					ret.m_wiresColor = float4(0, 0.8f, 0.8f, 1);
				}
			}
		}
		if (wires == false)
		{
			ret.m_drawFilled = true;
		}

		ret.m_drawNormals = object->renderNormals;

		const R_Mesh* res = ReadRes<R_Mesh>();

		ret.m_nIndices = res->m_nIndices;
		ret.m_nVertices = res->m_nVertices;

		ret.m_dataBuffer = res->id_data;
		ret.m_indicesBuffer = res->id_indices;

		ret.m_origin = res;
	}
	return ret;
}

const float3* mesh::GetVertices() const
{
	/*//Obtaining the vertices data from the buffer
	float3* ret = new float3[m_nVertices];
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3) * m_nVertices, ret);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	return ReadRes<R_Mesh>()->vertices;
}

const uint mesh::GetNumVertices()
{
	return ReadRes<R_Mesh>()->m_nVertices;
}

const uint* mesh::GetIndices() const
{
	return ReadRes<R_Mesh>()->indices;
}

const uint mesh::GetNumIndices()
{
	return ReadRes<R_Mesh>()->m_nIndices;
}

const float3 * mesh::GetNormals() const
{
	return ReadRes<R_Mesh>()->normals;
}

AABB mesh::GetAABB()
{
	return ReadRes<R_Mesh>()->aabb;
}

void mesh::EditorContent()
{
	const R_Mesh* res = ReadRes<R_Mesh>();
	char tmp[48];
	sprintf(tmp, "Wireframe##%llu", uid);
	ImGui::Checkbox(tmp, &wires);
	ImGui::NewLine();
	ImGui::Text("Resource: %s", res->name.data());

	ImGui::Text("Indices in memory: %i", res->indices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", res->id_indices);

	ImGui::Text("Vertices in memory: %i", res->vertices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	
	if (res->hasNormals)
	{
		ImGui::Text("Has normals");
	}
	if (res->hasUVs)
	{
		ImGui::Text("Has UVs");
	}
	ImGui::Separator();
	ImGui::Text("Texture index material:");
	sprintf(tmp, "##MaterialID%llu", uid);
	ImGui::InputInt(tmp, &texMaterialIndex);
}

void mesh::SaveSpecifics(pugi::xml_node& myNode)
{
	Resource* res = App->m_resourceManager->Peek(resource);
	myNode.append_attribute("res") = res->name.data();
	myNode.append_attribute("TextureIndex") = texMaterialIndex;
	myNode.append_attribute("Wired") = wires;
}

void mesh::LoadSpecifics(pugi::xml_node & myNode)
{
	std::string resName = myNode.attribute("res").as_string();
	resource = App->m_resourceManager->LinkResource(resName.data(), GetType());

	wires = myNode.attribute("Wired").as_bool();
	texMaterialIndex = myNode.attribute("TextureIndex").as_int();
	object->SetOriginalAABB();
}
