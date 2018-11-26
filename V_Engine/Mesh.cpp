#include "Mesh.h"

#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Mesh_RenderInfo.h"

#include "Application.h"

#include "R_Mesh.h"

//------------------------- MESH --------------------------------------------------------------------------------

Mesh::Mesh(std::string resource, Gameobject* linkedTo): ResourcedComponent(linkedTo, ComponentType::mesh)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Mesh##%i", m_uid);
	m_name = tmp;
	LoadResource(resource);
	std::vector<Mesh*> existingMeshes;
	m_gameObject->GetComponents<Mesh>(existingMeshes);
	m_textureIndex = existingMeshes.size();
}

Mesh_RenderInfo Mesh::GetMeshInfo() const
{
	Mesh_RenderInfo ret;
	if (m_gameObject->IsActive())
	{
		if (m_drawWired == true || GetOwner()->IsSelected())
		{
			ret.m_drawWired = true;
			if (m_drawWired == true)
			{
				ret.m_drawDoubleSidedFaces = true;
				ret.m_wiresColor = float4(0.f, 0.f, 0.f, 1.0f);
			}
			if (GetOwner()->IsSelected())
			{
				if (m_gameObject->GetTransform()->GetParent() != nullptr && m_gameObject->GetTransform()->GetParent()->GetGameobject()->IsSelected())
				{
					ret.m_wiresColor = float4(0, 0.5f, 0.5f, 1);
				}
				else
				{
					ret.m_wiresColor = float4(0, 0.8f, 0.8f, 1);
				}
			}
		}
		if (m_drawWired == false)
		{
			ret.m_drawFilled = true;
		}

		ret.m_drawNormals = m_drawNormals;

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

uint Mesh::GetNumVertices() const
{
	return ReadRes<R_Mesh>()->m_numVertices;
}

const uint* Mesh::GetIndices() const
{
	return ReadRes<R_Mesh>()->m_indices;
}

uint Mesh::GetNumIndices() const
{
	return ReadRes<R_Mesh>()->m_numIndices;
}

const float3 * Mesh::GetNormals() const
{
	return ReadRes<R_Mesh>()->m_normals;
}

AABB Mesh::GetAABB() const
{
	return ReadRes<R_Mesh>()->m_aabb;
}

void Mesh::EditorContent()
{
	const R_Mesh* res = ReadRes<R_Mesh>();
	char tmp[48];
	sprintf(tmp, "Wireframed##%llu", m_uid);
	ImGui::Checkbox(tmp, &m_drawWired);
	ImGui::SameLine();
	sprintf(tmp, "Normals##%llu", m_uid);
	ImGui::Checkbox(tmp, &m_drawNormals);
	ImGui::NewLine();
	ImGui::Text("Resource: %s", res->m_name.data());

	ImGui::Text("Indices in memory: %i", res->m_numIndices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", res->m_idIndices);

	ImGui::Text("Vertices in memory: %i", res->m_numVertices);
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
	ImGui::InputInt(tmp, &m_textureIndex);
}

void Mesh::SaveSpecifics(pugi::xml_node& myNode) const
{
	if (m_resource)
	{
		Resource* res = App->m_resourceManager->Peek(m_resource);
		myNode.append_attribute("res") = res->m_name.data();
		myNode.append_attribute("TextureIndex") = m_textureIndex;
		myNode.append_attribute("Wired") = m_drawWired;
	}
}

void Mesh::LoadSpecifics(pugi::xml_node & myNode)
{
	{
		std::string resName = myNode.attribute("res").as_string();
		m_resource = App->m_resourceManager->LinkResource(resName.data(), GetType());

		m_drawWired = myNode.attribute("Wired").as_bool();
		m_textureIndex = myNode.attribute("TextureIndex").as_int();
	}
}

bool Mesh::RayCast(const LineSegment & ray, float3 * OUT_collisionPoint, float3 * OUT_collisionNormal)
{
	float collisionDistance = floatMax;
	bool collided = false;
	LineSegment transformedRay = ray;
	transformedRay.Transform(GetOwner()->GetTransform()->GetGlobalTransform().InverseTransposed());
	//Generating the triangles the mes has, and checking them one by one
	const float3* vertices = GetVertices();
	const uint* index = GetIndices();
	for (int n = 0; n < GetNumIndices(); n += 3)
	{
		Triangle tri(vertices[index[n]], vertices[index[n + 1]], vertices[index[n + 2]]);
		float3 intersectionPoint;
		float distance;
		//If the triangle we collided with is further away than a previous collision, we'll ignore it
		if (tri.Intersects(transformedRay, &distance, &intersectionPoint) == true)
		{
			collided = true;
			if (distance < collisionDistance)
			{
				collisionDistance = distance;
				float3 meshSpacePosition = intersectionPoint;
				float3 meshSpaceNormal = tri.NormalCCW();
				LineSegment positionWithNormal(meshSpacePosition, meshSpacePosition + meshSpaceNormal);
				positionWithNormal.Transform(GetOwner()->GetTransform()->GetGlobalTransform().Transposed());
				*OUT_collisionPoint = positionWithNormal.a;
				*OUT_collisionNormal = positionWithNormal.b - positionWithNormal.a;
			}
		}
	}
	return collided;
}
