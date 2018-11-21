#include "GameObject.h"

#include "AllComponents.h"

#include "imGUI\imgui.h"

#include "OpenGL.h"
#include <map>

#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleGOmanager.h"
#include "ModuleEditor.h"


Gameobject::Gameobject()
	: Gameobject(GenerateUUID())
{ }

Gameobject::Gameobject(uint64_t uid)
	: m_transform(this)
	, m_uid(uid)
	, m_name("Unnamed")
{
	//TODO: To fix, init being called before M-GoManager is assigned
	if (App->m_goManager)
	{
		App->m_goManager->m_dynamicGO.push_back(this);
	}
}

Gameobject::~Gameobject()
{
	if (IsSelected())
	{
		App->m_editor->UnselectGameObject();
	}
	for(auto component : m_components)
	{
		App->m_goManager->DeleteComponent(component);
	}

	std::vector<Transform*> childs = GetTransform()->GetChilds();
	if (childs.empty() == false)
	{
		for (auto child : childs)
		{
			delete child->GetGameobject();
		}
	}
}

//TODO distribute this on every component
void Gameobject::DrawAttributeEditorContent()
{
	if (ImGui::BeginPopup("Add Component"))
	{
		if (ImGui::BeginMenu("Mesh##add"))
		{
			std::vector<std::pair<std::string, std::vector<std::string>>> meshRes = App->m_resourceManager->GetAvaliableResources(ComponentType::mesh);
			std::vector<std::pair<std::string, std::vector<std::string>>>::iterator fileIt = meshRes.begin();
			for (; fileIt != meshRes.end(); fileIt++)
			{
				if (ImGui::BeginMenu(fileIt->first.data()))
				{
					std::vector<std::string>::iterator it = fileIt->second.begin();
					for (; it != fileIt->second.end(); it++)
					{
						if (ImGui::MenuItem(it->data()))
						{
							CreateComponent(ComponentType::mesh, *it);
							break;
						}
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Material##add"))
		{
			if (HasComponent<Material>())
			{
				ImGui::Text("Already has a material!");
			}
			else
			{
				std::vector<std::pair<std::string, std::vector<std::string>>> meshRes = App->m_resourceManager->GetAvaliableResources(ComponentType::material);
				std::vector<std::pair<std::string, std::vector<std::string>>>::iterator fileIt = meshRes.begin();
				for (; fileIt != meshRes.end(); fileIt++)
				{
					if (ImGui::BeginMenu(fileIt->first.data()))
					{
						std::vector<std::string>::iterator it = fileIt->second.begin();
						for (; it != fileIt->second.end(); it++)
						{
							if (ImGui::MenuItem(it->data()))
							{
								CreateComponent(ComponentType::material, *it);
								break;
							}
						}
						ImGui::EndMenu();
					}
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Camera##add"))
		{
			CreateComponent(ComponentType::camera);
		}
		if (HasComponent<Billboard>() == false)
		{
			if (ImGui::MenuItem("Billboard##add"))
			{
				CreateComponent(ComponentType::billboard);
			}
		}
		ImGui::EndPopup();
	}

	bool isActive = IsActive();
	ImGui::Checkbox("", &isActive);
	if (isActive != IsActive())
	{
		SetActive(isActive);
	}
	ImGui::SameLine();
	ImGui::Text("Name:");
	ImGui::SameLine();
	ImGui::InputText("##Name", m_name, NAME_MAX_LEN);

	if (ImGui::Button("Add:"))
	{
		ImGui::OpenPopup("Add Component");
	}
	
	if (ImGui::CollapsingHeader("Transform"))
	{
		m_transform.EditorContent();
	}
	for (std::vector<Component*>::iterator it = m_components.begin(); it != m_components.end(); it++)
	{
		(*it)->DrawOnEditor();
	}
}

void Gameobject::DrawAABB() const
{
	if (Time.PlayMode != PlayMode::Play)
	{
		AABB aabb = GetAABB();
		if (aabb.IsFinite())
		{
			math::float3 corners[8];
			aabb.GetCornerPoints(corners);
			App->m_renderer3D->DrawBox(corners);
		}
	}
}

void Gameobject::DrawOBB() const
{
	if (Time.PlayMode != PlayMode::Play)
	{
		OBB obb = GetOBB();
		if (obb.IsFinite())
		{
			math::float3 corners[8];
			obb.GetCornerPoints(corners);
			App->m_renderer3D->DrawBox(corners, float4(0.2f, 0.45f, 0.27f, 1.0f));
		}
	}
}

void Gameobject::PositionChanged()
{
	GetTransform()->UpdateGlobalTransform();

	//Updating cameras position
	for (auto component : m_components)
	{
		component->PositionChanged();
	}

	for (auto child : GetTransform()->GetChilds())
	{
		child->GetGameobject()->PositionChanged();
	}
}

void Gameobject::SetActive(bool state, bool justPublic)
{
	if (state == m_publicActive)
	{
		return;
	}
	m_publicActive = state;

	for (auto child : GetTransform()->GetChilds())
	{
		child->GetGameobject()->SetActive(state, true);
	}

	if (justPublic)
	{
		return;
	}
	m_active = state;

	if (state == true && GetTransform()->GetParent() != nullptr)
	{
		GetTransform()->GetParent()->GetGameobject()->SetActive(true);
	}
}

bool Gameobject::IsActive() const
{
	if (m_active == false)
	{
		return false;
	}
	return m_publicActive;
}


void Gameobject::SetName(const char * newName)
{
	strcpy(m_name, newName);
}

const char * Gameobject::GetName() const
{
	return m_name;
}

Component* Gameobject::CreateComponent(ComponentType type, std::string res, bool forceCreation)
{
	Component* toAdd = nullptr;
	switch (type)
	{
	case ComponentType::mesh:
	{
		toAdd = new Mesh(res, this);
		break;
	}
	case ComponentType::material:
	{
		if (HasComponent<Material>() == false)
		{
			toAdd = new Material(res, this);
		}
		break;
	}
	case ComponentType::camera:
	{
		toAdd = new Camera(this);
		break;
	}
	case ComponentType::billboard:
	{
		if (HasComponent<Billboard>() == false)
		{
			toAdd = new Billboard(this);
		}
		break;
	}
	}

	if (toAdd != nullptr)
	{
		if (toAdd->IsResourceMissing() == false || forceCreation)
		{
			m_components.push_back(toAdd);
			App->m_goManager->m_components.insert(std::pair<ComponentType, Component*>(toAdd->GetType(), toAdd));
		}
		else
		{
			LOG("Error loading a component from %s", toAdd->m_name.data());
			RELEASE(toAdd);
		}
	}

	return toAdd;
}

Transform* Gameobject::GetTransform()
{
	return &m_transform;
}

const Transform* Gameobject::GetTransform() const
{
	return &m_transform;
}

void Gameobject::Save(pugi::xml_node& node) const
{
	if (GetTransform()->m_hiddenOnOutliner == false)
	{
		pugi::xml_node GO = node.append_child("GO");
		GO.append_attribute("Active") = m_active;
		GO.append_attribute("m_name") = m_name;
		if (GetTransform()->GetParent() != nullptr)
		{
			GO.append_attribute("UID") = m_uid;
			if (GetTransform()->GetParent()->GetParent() != nullptr)
			{
				GO.append_attribute("m_parent") = GetTransform()->GetParent()->GetGameobject()->GetUID();
			}
			else
			{
				GO.append_attribute("m_parent") = 0;
			}
		}
		else
		{
			GO.append_attribute("UID") = 0;
			GO.append_attribute("m_parent") = "0";
		}

		m_transform.SaveSpecifics(GO.append_child("Transform"));

		for (auto child : GetTransform()->GetChilds())
		{
			child->GetGameobject()->Save(node);
		}
	}
}

void Gameobject::RemoveComponent(Component * comp)
{
	auto it = std::find(m_components.begin(), m_components.end(), comp);
	if (it != m_components.end())
	{
		App->m_goManager->DeleteComponent(comp);
	}
}

AABB Gameobject::GetObjectSpaceAABB() const
{
	AABB objectSpaceAABB;
	objectSpaceAABB.SetNegativeInfinity();
	std::vector<Mesh*> meshes;
	GetComponents<Mesh>(meshes);
	if (meshes.empty() == false)
	{
		for (auto mesh : meshes)
		{
			objectSpaceAABB.Enclose(mesh->GetAABB());
		}
	}
	return objectSpaceAABB;
}

AABB Gameobject::GetAABB() const
{
	return GetOBB().MinimalEnclosingAABB();
}

OBB Gameobject::GetOBB() const
{
	AABB objectAABB = GetObjectSpaceAABB();
	if (objectAABB.minPoint.x != -inf && objectAABB.minPoint.y != -inf && objectAABB.minPoint.z != -inf)
	{
		return GetObjectSpaceAABB().Transform(GetTransform()->GetGlobalTransform().Transposed());
	}
	return OBB(AABB(float3(0,0,0), float3(0,0,0)));
}

bool Gameobject::IsSelected() const
{
	return (App->m_editor->GetSelectedGameObject() == this);
}
