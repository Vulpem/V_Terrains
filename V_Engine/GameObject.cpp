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


GameObject::GameObject()
	: m_transform(this)
{
	m_uid = GenerateUUID();
	m_aabb.SetNegativeInfinity();
	m_originalAABB.SetNegativeInfinity();
	strcpy(m_name, "Unnamed");
	App->m_goManager->m_dynamicGO.push_back(this);
}

GameObject::GameObject(uint64_t Uid)
	: m_transform(this)
{
	m_uid = Uid;
	m_aabb.SetNegativeInfinity();
	m_originalAABB.SetNegativeInfinity();
	strcpy(m_name, "Unnamed");
	App->m_goManager->m_dynamicGO.push_back(this);
}

GameObject::~GameObject()
{
	if (IsStatic() == false)
	{
		if (App->m_goManager->m_dynamicGO.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = App->m_goManager->m_dynamicGO.begin(); it != App->m_goManager->m_dynamicGO.end(); it++)
			{
				if ((*it) == this)
				{
					App->m_goManager->m_dynamicGO.erase(it);
					break;
				}
			}
		}
	}
	else
	{
		App->m_goManager->m_quadTree.Remove(this);
	}

	GetTransform()->SetParent(nullptr);

	std::vector<Component*>::reverse_iterator comp = m_components.rbegin();
	while (comp != m_components.rend())
	{
		std::multimap<ComponentType, Component*>::iterator it = App->m_goManager->m_components.find((*comp)->GetType());
		for (; it->first == (*comp)->GetType(); it++)
		{
			if (it->second->GetUID() == (*comp)->GetUID())
			{
				App->m_goManager->m_components.erase(it);
				break;
			}
		}

		delete *comp;
		comp++;
	}
	m_components.clear();

	std::vector<Transform*> childs = GetTransform()->GetChilds();
	if (childs.empty() == false)
	{
		std::vector<Transform*>::iterator iterator = childs.begin();
		for (auto child : childs)
		{
			delete child->GetGameobject();
		}
	}
}

void GameObject::DrawOnEditor()
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
							AddComponent(ComponentType::mesh, *it);
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
								AddComponent(ComponentType::material, *it);
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
			AddComponent(ComponentType::camera);
		}
		if (HasComponent<Billboard>() == false)
		{
			if (ImGui::MenuItem("Billboard##add"))
			{
				AddComponent(ComponentType::billboard);
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
	ImGui::SameLine();
	ImGui::Text("m_static: ");
	ImGui::SameLine();
	bool isStatic = m_static;
	ImGui::Checkbox("##isObjectStatic", &isStatic);
	if (isStatic != m_static && App->m_goManager->m_setting == nullptr)
	{
		if (GetTransform()->GetChilds().empty() == true)
		{
			App->m_goManager->SetStatic(isStatic, this);
		}
		else
		{
			App->m_goManager->m_setting = this;
			App->m_goManager->m_settingStatic = isStatic;
		}
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

void GameObject::DrawLocator()
{
	if (Time.PlayMode != PlayMode::Play)
	{
		float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f);
		if (m_selected)
		{
			if (GetTransform()->GetParent()->GetGameobject()->m_selected)
			{
				color = float4(0, 0.5f, 0.5f, 1);
			}
			else {
				color = float4(0, 0.8f, 0.8f, 1);
			}
		}
		App->m_renderer3D->DrawLocator(GetTransform()->GetGlobalTransform(), color);

		std::vector<Transform*> childs = GetTransform()->GetChilds();
		for (auto child : childs)
		{
			if (child->GetGameobject()->HasComponent<Mesh>() == false)
			{
				math::float3 childPos(child->GetGlobalPos());
				App->m_renderer3D->DrawLine(GetTransform()->GetGlobalPos(), childPos, color);
			}
		}
	}
}

void GameObject::DrawAABB()
{
	if (Time.PlayMode != PlayMode::Play)
	{
		if (m_aabb.IsFinite())
		{
			math::float3 corners[8];
			m_aabb.GetCornerPoints(corners);
			App->m_renderer3D->DrawBox(corners);
		}
	}
}

void GameObject::DrawOBB()
{
	if (Time.PlayMode != PlayMode::Play)
	{
		if (m_obb.IsFinite())
		{
			math::float3 corners[8];
			m_obb.GetCornerPoints(corners);
			App->m_renderer3D->DrawBox(corners, float4(0.2f, 0.45f, 0.27f, 1.0f));
		}
	}
}

void GameObject::Select(bool _renderNormals)
{
	m_selected = true;
	m_drawNormals = _renderNormals;

	GetTransform()->UpdateEditorValues();

	std::vector<Transform*> childs = GetTransform()->GetChilds();
	for (auto child : childs)
	{
		child->GetGameobject()->Select();
	}
}

void GameObject::Unselect()
{
	m_selected = false;
	m_drawNormals = false;
	std::vector<Transform*> childs = GetTransform()->GetChilds();
	for (auto child : childs)
	{
		child->GetGameobject()->Unselect();
	}
}

void GameObject::SetOriginalAABB()
{
	if (HasComponent<Mesh>())
	{
		m_originalAABB.SetNegativeInfinity();
		std::vector<Mesh*> meshes;
		meshes.reserve(2);
		GetComponents<Mesh>(meshes);

		for (std::vector<Mesh*>::iterator it = meshes.begin(); it != meshes.end(); it++)
		{
			m_originalAABB.Enclose((*it)->GetAABB().maxPoint);
			m_originalAABB.Enclose((*it)->GetAABB().minPoint);
		}
	}
	else
	{
		m_originalAABB.minPoint = float3{ -0.25f,-0.25f,-0.25f };
		m_originalAABB.maxPoint = float3{ 0.25f,0.25f,0.25f };
	}

	UpdateAABB();
}

void GameObject::UpdateAABB()
{
	m_aabb.SetNegativeInfinity();
	m_obb.SetNegativeInfinity();
	if (m_originalAABB.IsFinite())
	{
		m_obb = m_originalAABB;
		m_obb.Transform(GetTransform()->GetGlobalTransform().Transposed());
		m_aabb.Enclose(m_obb);
	}
}

void GameObject::UpdateTransformMatrix()
{
	GetTransform()->UpdateGlobalTransform();

	UpdateAABB();

	//Updating cameras position
	if (HasComponent<Camera>())
	{
		std::vector<Camera*> cams;
		GetComponents<Camera>(cams);
		std::vector<Camera*>::iterator it = cams.begin();
		while (it != cams.end())
		{
			(*it)->UpdateCamMatrix();
			it++;
		}
	}

	for (auto child : GetTransform()->GetChilds())
	{
		child->GetGameobject()->UpdateTransformMatrix();
	}
}

void GameObject::SetActive(bool state, bool justPublic)
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

bool GameObject::IsActive()
{
	if (m_active == false)
	{
		return false;
	}
	return m_publicActive;
}


void GameObject::SetName(const char * newName)
{
	strcpy(m_name, newName);
}

const char * GameObject::GetName()
{
	return m_name;
}

Component* GameObject::AddComponent(ComponentType type, std::string res, bool forceCreation)
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
		if (toAdd->MissingComponent() == false || forceCreation)
		{
			m_components.push_back(toAdd);
			App->m_goManager->m_components.insert(std::pair<ComponentType, Component*>(toAdd->GetType(), toAdd));
			if (toAdd->GetType() == ComponentType::mesh)
			{
				SetOriginalAABB();
			}
		}
		else
		{
			LOG("Error loading a component from %s", toAdd->m_name.data());
			RELEASE(toAdd);
		}
	}

	return toAdd;
}

Transform* GameObject::GetTransform()
{
	return &m_transform;
}

void GameObject::Delete()
{
	App->m_editor->UnselectGameObject(this);
	App->m_goManager->DeleteGameObject(this);
}

void GameObject::Save(pugi::xml_node& node)
{
	if (m_hiddenOnOutliner == false)
	{
		pugi::xml_node GO = node.append_child("GO");
		GO.append_attribute("Active") = m_active;
		GO.append_attribute("m_static") = m_static;
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

void GameObject::RemoveComponent(Component * comp)
{
	std::vector<Component*>::iterator it = m_components.begin();
	for (; it != m_components.end(); it++)
	{
		if ((*it) == comp)
		{
			std::map<ComponentType, Component*>::iterator mapIt = App->m_goManager->m_components.find((*it)->GetType());
			for (; mapIt != App->m_goManager->m_components.end() && mapIt->first == (*it)->GetType(); mapIt++)
			{
				if (mapIt->second == comp)
				{
					App->m_goManager->m_components.erase(mapIt);
					break;
				}
			}
			RELEASE(*it);
			m_components.erase(it);
			return;
		}
	}
}
