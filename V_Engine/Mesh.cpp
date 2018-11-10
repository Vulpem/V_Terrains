#include "Mesh.h"

#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Mesh_RenderInfo.h"

#include "Application.h"

#include "R_Mesh.h"

//------------------------- MESH --------------------------------------------------------------------------------

Mesh::Mesh(std::string resource, GameObject* linkedTo): ResourcedComponent(linkedTo, ComponentType::mesh)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Mesh##%i", m_uid);
	m_name = tmp;
	LoadResource(resource);
	texMaterialIndex = m_gameObject->AmountOfComponent(ComponentType::mesh);
}

Mesh_RenderInfo Mesh::GetMeshInfo()
{
	Mesh_RenderInfo ret;
	if (m_gameObject->IsActive())
	{
		if (wires == true || m_gameObject->m_selected)
		{
			ret.m_drawWired = true;
			if (wires == true)
			{
				ret.m_drawDoubleSidedFaces = true;
				ret.m_wiresColor = float4(0.f, 0.f, 0.f, 1.0f);
			}
			if (m_gameObject->m_selected)
			{
				if (m_gameObject->m_parent && m_gameObject->m_parent->m_selected)
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

		ret.m_drawNormals = m_gameObject->m_drawNormals;

		const R_Mesh* res = ReadRes<R_Mesh>();

		ret.m_nIndices = res->m_numIndices;
		ret.m_nVertices = res->m_numVertices;

		ret.m_dataBuffer = res->m_idData;
		ret.m_indicesBuffer = res->m_idIndices;

		ret.m_origin = res;
	}
	return ret;
}

const float3* Mesh::GetVertices() const
{
	/*//Obtaining the vertices data from the buffer
	float3* ret = new float3[m_nVertices];
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3) * m_nVertices, ret);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	return ReadRes<R_Mesh>()->m_vertices;
}

const uint Mesh::GetNumVertices()
{
	return ReadRes<R_Mesh>()->m_numVertices;
}

const uint* Mesh::GetIndices() const
{
	return ReadRes<R_Mesh>()->m_indices;
}

const uint Mesh::GetNumIndices()
{
	return ReadRes<R_Mesh>()->m_numIndices;
}

const float3 * Mesh::GetNormals() const
{
	return ReadRes<R_Mesh>()->m_normals;
}

AABB Mesh::GetAABB()
{
	return ReadRes<R_Mesh>()->m_aabb;
}

void Mesh::EditorContent()
{
	const R_Mesh* res = ReadRes<R_Mesh>();
	char tmp[48];
	sprintf(tmp, "Wireframe##%llu", m_uid);
	ImGui::Checkbox(tmp, &wires);
	ImGui::NewLine();
	ImGui::Text("Resource: %s", res->m_name.data());

	ImGui::Text("Indices in memory: %i", res->m_indices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", res->m_idIndices);

	ImGui::Text("Vertices in memory: %i", res->m_vertices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	
	if (res->m_hasNormals)
	{
		ImGui::Text("Has normals");
	}
	if (res->m_hasUVs)
	{
		ImGui::Text("Has UVs");
	}
	ImGui::Separator();
	ImGui::Text("Texture index material:");
	sprintf(tmp, "##MaterialID%llu", m_uid);
	ImGui::InputInt(tmp, &texMaterialIndex);
}

void Mesh::SaveSpecifics(pugi::xml_node& myNode)
{
	if (m_resource)
	{
		Resource* res = App->m_resourceManager->Peek(m_resource);
		myNode.append_attribute("res") = res->m_name.data();
		myNode.append_attribute("TextureIndex") = texMaterialIndex;
		myNode.append_attribute("Wired") = wires;
	}
}

void Mesh::LoadSpecifics(pugi::xml_node & myNode)
{
	{
		std::string resName = myNode.attribute("res").as_string();
		m_resource = App->m_resourceManager->LinkResource(resName.data(), GetType());

		wires = myNode.attribute("Wired").as_bool();
		texMaterialIndex = myNode.attribute("TextureIndex").as_int();
		m_gameObject->SetOriginalAABB();
	}
}
