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
{
	m_uid = GenerateUUID();
	for (int n = 0; n < m_nComponentTypes; n++)
	{
		m_hasComponents[(ComponentType)n] = 0;
	}
	m_aabb.SetNegativeInfinity();
	m_originalAABB.SetNegativeInfinity();
	strcpy(m_name, "Unnamed");
	App->m_goManager->dynamicGO.push_back(this);
}

GameObject::GameObject(uint64_t Uid)
{
	m_uid = Uid;
	for (int n = 0; n < m_nComponentTypes; n++)
	{
		m_hasComponents[(ComponentType)n] = 0;
	}
	m_aabb.SetNegativeInfinity();
	m_originalAABB.SetNegativeInfinity();
	strcpy(m_name, "Unnamed");
	App->m_goManager->dynamicGO.push_back(this);
}

GameObject::~GameObject()
{
	if (IsStatic() == false)
	{
		if (App->m_goManager->dynamicGO.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = App->m_goManager->dynamicGO.begin(); it != App->m_goManager->dynamicGO.end(); it++)
			{
				if ((*it) == this)
				{
					App->m_goManager->dynamicGO.erase(it);
					break;
				}
			}
		}
	}
	else
	{
		App->m_goManager->quadTree.Remove(this);
	}

	if (m_parent != nullptr)
	{
		std::vector<GameObject*>::iterator it = m_parent->m_childs.begin();
		while ((*it) != this)
		{
			it++;
		}
		m_parent->m_childs.erase(it);
	}

	std::vector<Component*>::reverse_iterator comp = m_components.rbegin();
	while (comp != m_components.rend())
	{
		std::multimap<ComponentType, Component*>::iterator it = App->m_goManager->components.find((*comp)->GetType());
		for (; it->first == (*comp)->GetType(); it++)
		{
			if (it->second->GetUID() == (*comp)->GetUID())
			{
				App->m_goManager->components.erase(it);
				break;
			}
		}

		delete *comp;
		comp++;
	}
	m_components.clear();

	if (m_childs.empty() == false)
	{
		std::vector<GameObject*>::iterator iterator = m_childs.begin();
		while (m_childs.size() > 0 && iterator != m_childs.end())
		{
			delete (*iterator);
			//Erasing a Node will already remove it from the child list in its destructor, so we don't have to empty the list here, it will be done automatically
			if (m_childs.size() > 0)
			{
				iterator = m_childs.begin();
			}
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
			if (HasComponent(ComponentType::material))
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
		if (HasComponent(ComponentType::billboard) == false)
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
	if (isStatic != m_static && App->m_goManager->setting == nullptr)
	{
		if (m_childs.empty() == true)
		{
			App->m_goManager->SetStatic(isStatic, this);
		}
		else
		{
			App->m_goManager->setting = this;
			App->m_goManager->settingStatic = isStatic;
		}
	}

	for (std::vector<Component*>::iterator it = m_components.begin(); it != m_components.end(); it++)
	{
		(*it)->DrawOnEditor();
	}
}

void GameObject::DrawLocator()
{
	if (Time.PlayMode != Play::Play)
	{
		float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f);
		if (m_selected)
		{
			if (m_parent->m_selected)
			{
				color = float4(0, 0.5f, 0.5f, 1);
			}
			else {
				color = float4(0, 0.8f, 0.8f, 1);
			}
		}
		App->m_renderer3D->DrawLocator(GetTransform()->GetGlobalTransform(), color);

		if (m_childs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = m_childs.begin(); it != m_childs.end(); it++)
			{
				if ((*it)->HasComponent(ComponentType::transform) && !(*it)->HasComponent(ComponentType::mesh))
				{
					math::float3 childPos((*it)->GetTransform()->GetGlobalPos());
					App->m_renderer3D->DrawLine(GetTransform()->GetGlobalPos(), childPos, color);
				}
			}
		}
	}
}

void GameObject::DrawAABB()
{
	if (Time.PlayMode != Play::Play)
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
	if (Time.PlayMode != Play::Play)
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

	std::vector<GameObject*>::iterator childIt = m_childs.begin();
	while (childIt != m_childs.end())
	{
		(*childIt)->Select();
		childIt++;
	}
}

void GameObject::Unselect()
{
	m_selected = false;
	m_drawNormals = false;
	if (m_childs.empty() == false)
	{
		std::vector<GameObject*>::iterator childIt = m_childs.begin();
		while (childIt != m_childs.end())
		{
			(*childIt)->Unselect();
			childIt++;
		}
	}
}

void GameObject::SetOriginalAABB()
{
	if (HasComponent(ComponentType::mesh))
	{
		m_originalAABB.SetNegativeInfinity();
		std::vector<Mesh*> meshes = GetComponent<Mesh>();

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
	if (HasComponent(ComponentType::transform))
	{	
		GetTransform()->UpdateGlobalTransform();
	}

	UpdateAABB();

	//Updating cameras position
	if (HasComponent(ComponentType::camera))
	{
		std::vector<Camera*> cams = GetComponent<Camera>();
		std::vector<Camera*>::iterator it = cams.begin();
		while (it != cams.end())
		{
			(*it)->UpdateCamMatrix();
			it++;
		}
	}

	if (m_childs.empty() == false)
	{
		std::vector<GameObject*>::iterator child = m_childs.begin();
		while (child != m_childs.end())
		{
			(*child)->UpdateTransformMatrix();
			child++;
		}
	}

}

void GameObject::SetActive(bool state, bool justPublic)
{
	if (state == m_publicActive)
	{
		return;
	}
	m_publicActive = state;

	std::vector<GameObject*>::iterator childIt = m_childs.begin();
	while (childIt != m_childs.end())
	{
		(*childIt)->SetActive(state, true);
		childIt++;
	}

	if (justPublic)
	{
		return;
	}
	m_active = state;

	if (state == true && m_parent)
	{
		m_parent->SetActive(true);
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
	case ComponentType::transform:
	{
		if (HasComponent(ComponentType::transform) == false)
		{
			toAdd = new Transform(this);
			m_transform = (Transform*)toAdd;
		}
		break;
	}
	case ComponentType::mesh:
	{
		toAdd = new Mesh(res, this);
		break;
	}
	case ComponentType::material:
	{
		if (HasComponent(ComponentType::material) == false)
		{
			toAdd = new Material(res, this);
		}
		break;
	}
	case ComponentType::camera:
	{
		if (HasComponent(ComponentType::transform))
		{
			toAdd = new Camera(this);
		}
		break;
	}
	case ComponentType::billboard:
	{
		if (HasComponent(ComponentType::billboard) == false)
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
			m_hasComponents[toAdd->GetType()] += 1;
			m_components.push_back(toAdd);
			App->m_goManager->components.insert(std::pair<ComponentType, Component*>(toAdd->GetType(), toAdd));
			if (toAdd->GetType() == ComponentType::mesh)
			{
				SetOriginalAABB();
			}
		}
		else
		{
			LOG("Error loading a component from %s", toAdd->name.data());
			RELEASE(toAdd);
		}
	}

	return toAdd;
}

bool GameObject::HasComponent(ComponentType type)
{
	return m_hasComponents[type] != 0;
}

uint GameObject::AmountOfComponent(ComponentType type)
{
	return m_hasComponents[type];
}

Transform * GameObject::GetTransform()
{
	return m_transform;
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
		if (m_parent != nullptr)
		{
			GO.append_attribute("UID") = m_uid;
			if (m_parent->m_parent != nullptr)
			{
				GO.append_attribute("m_parent") = m_parent->GetUID();
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

		for (std::vector<GameObject*>::iterator it = m_childs.begin(); it != m_childs.end(); it++)
		{
			(*it)->Save(node);
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
			std::map<ComponentType, Component*>::iterator mapIt = App->m_goManager->components.find((*it)->GetType());
			for (; mapIt != App->m_goManager->components.end() && mapIt->first == (*it)->GetType(); mapIt++)
			{
				if (mapIt->second == comp)
				{
					App->m_goManager->components.erase(mapIt);
					break;
				}
			}
			m_hasComponents[(*it)->GetType()] -= 1;
			RELEASE(*it);
			m_components.erase(it);
			return;
		}
	}
}
