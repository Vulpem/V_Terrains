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
	uid = GenerateUUID();
	for (int n = 0; n < nComponentTypes; n++)
	{
		HasComponents[n] = false;
	}
	aabb.SetNegativeInfinity();
	originalAABB.SetNegativeInfinity();
	strcpy(name, "Unnamed");
	App->m_goManager->dynamicGO.push_back(this);
}

GameObject::GameObject(uint64_t Uid)
{
	uid = Uid;
	for (int n = 0; n < nComponentTypes; n++)
	{
		HasComponents[n] = false;
	}
	aabb.SetNegativeInfinity();
	originalAABB.SetNegativeInfinity();
	strcpy(name, "Unnamed");
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

	if (parent != nullptr)
	{
		std::vector<GameObject*>::iterator it = parent->childs.begin();
		while ((*it) != this)
		{
			it++;
		}
		parent->childs.erase(it);
	}

	std::vector<Component*>::reverse_iterator comp = components.rbegin();
	while (comp != components.rend())
	{
		std::multimap<Component::Type, Component*>::iterator it = App->m_goManager->components.find((*comp)->GetType());
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
	components.clear();

	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator iterator = childs.begin();
		while (childs.size() > 0 && iterator != childs.end())
		{
			delete (*iterator);
			//Erasing a Node will already remove it from the child list in its destructor, so we don't have to empty the list here, it will be done automatically
			if (childs.size() > 0)
			{
				iterator = childs.begin();
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
			std::vector<std::pair<std::string, std::vector<std::string>>> meshRes = App->m_resourceManager->GetAvaliableResources(Component::Type::C_mesh);
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
							AddComponent(Component::Type::C_mesh, *it);
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
			if (HasComponent(Component::Type::C_material))
			{
				ImGui::Text("Already has a material!");
			}
			else
			{
				std::vector<std::pair<std::string, std::vector<std::string>>> meshRes = App->m_resourceManager->GetAvaliableResources(Component::Type::C_material);
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
								AddComponent(Component::Type::C_material, *it);
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
			AddComponent(Component::Type::C_camera);
		}
		if (HasComponent(Component::Type::C_Billboard) == false)
		{
			if (ImGui::MenuItem("Billboard##add"))
			{
				AddComponent(Component::Type::C_Billboard);
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
	ImGui::InputText("##Name", name, NAME_MAX_LEN);

	if (ImGui::Button("Add:"))
	{
		ImGui::OpenPopup("Add Component");
	}
	ImGui::SameLine();
	ImGui::Text("Static: ");
	ImGui::SameLine();
	bool isStatic = Static;
	ImGui::Checkbox("##isObjectStatic", &isStatic);
	if (isStatic != Static && App->m_goManager->setting == nullptr)
	{
		if (childs.empty() == true)
		{
			App->m_goManager->SetStatic(isStatic, this);
		}
		else
		{
			App->m_goManager->setting = this;
			App->m_goManager->settingStatic = isStatic;
		}
	}

	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		(*it)->DrawOnEditor();
	}
}

void GameObject::DrawLocator()
{
	if (Time.PlayMode != Play::Play)
	{
		float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f);
		if (selected)
		{
			if (parent->selected)
			{
				color = float4(0, 0.5f, 0.5f, 1);
			}
			else {
				color = float4(0, 0.8f, 0.8f, 1);
			}
		}
		App->m_renderer3D->DrawLocator(GetTransform()->GetGlobalTransform(), color);

		if (childs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
			{
				if ((*it)->HasComponent(Component::Type::C_transform) && !(*it)->HasComponent(Component::Type::C_mesh))
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
		if (aabb.IsFinite())
		{
			math::float3 corners[8];
			aabb.GetCornerPoints(corners);
			App->m_renderer3D->DrawBox(corners);
		}
	}
}

void GameObject::DrawOBB()
{
	if (Time.PlayMode != Play::Play)
	{
		if (obb.IsFinite())
		{
			math::float3 corners[8];
			obb.GetCornerPoints(corners);
			App->m_renderer3D->DrawBox(corners, float4(0.2f, 0.45f, 0.27f, 1.0f));
		}
	}
}

void GameObject::Select(bool _renderNormals)
{
	selected = true;
	renderNormals = _renderNormals;

	GetTransform()->UpdateEditorValues();

	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Select();
		childIt++;
	}
}

void GameObject::Unselect()
{
	selected = false;
	renderNormals = false;
	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator childIt = childs.begin();
		while (childIt != childs.end())
		{
			(*childIt)->Unselect();
			childIt++;
		}
	}
}

void GameObject::SetOriginalAABB()
{
	if (HasComponent(Component::Type::C_mesh))
	{
		originalAABB.SetNegativeInfinity();
		std::vector<mesh*> meshes = GetComponent<mesh>();

		for (std::vector<mesh*>::iterator it = meshes.begin(); it != meshes.end(); it++)
		{
			originalAABB.Enclose((*it)->GetAABB().maxPoint);
			originalAABB.Enclose((*it)->GetAABB().minPoint);
		}
	}
	else
	{
		originalAABB.minPoint = float3{ -0.25f,-0.25f,-0.25f };
		originalAABB.maxPoint = float3{ 0.25f,0.25f,0.25f };
	}

	UpdateAABB();
}

void GameObject::UpdateAABB()
{
	aabb.SetNegativeInfinity();
	obb.SetNegativeInfinity();
	if (originalAABB.IsFinite())
	{
		obb = originalAABB;
		obb.Transform(GetTransform()->GetGlobalTransform().Transposed());
		aabb.Enclose(obb);
	}
}

void GameObject::UpdateTransformMatrix()
{
	if (HasComponent(Component::Type::C_transform))
	{	
		GetTransform()->UpdateGlobalTransform();
	}

	UpdateAABB();

	//Updating cameras position
	if (HasComponent(Component::Type::C_camera))
	{
		std::vector<Camera*> cams = GetComponent<Camera>();
		std::vector<Camera*>::iterator it = cams.begin();
		while (it != cams.end())
		{
			(*it)->UpdateCamMatrix();
			it++;
		}
	}

	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator child = childs.begin();
		while (child != childs.end())
		{
			(*child)->UpdateTransformMatrix();
			child++;
		}
	}

}

void GameObject::SetActive(bool state, bool justPublic)
{
	if (state == publicActive)
	{
		return;
	}
	publicActive = state;

std::vector<GameObject*>::iterator childIt = childs.begin();
while (childIt != childs.end())
{
	(*childIt)->SetActive(state, true);
	childIt++;
}

if (justPublic)
{
	return;
}
active = state;

if (state == true && parent)
{
	parent->SetActive(true);
}
}

bool GameObject::IsActive()
{
	if (active == false)
	{
		return false;
	}
	return publicActive;
}


void GameObject::SetName(const char * newName)
{
	strcpy(name, newName);
}

const char * GameObject::GetName()
{
	return name;
}

Component* GameObject::AddComponent(Component::Type type, std::string res, bool forceCreation)
{
	Component* toAdd = nullptr;
	switch (type)
	{
	case Component::Type::C_transform:
	{
		if (HasComponent(Component::C_transform) == false)
		{
			toAdd = new Transform(this);
			transform = (Transform*)toAdd;
		}
		break;
	}
	case Component::Type::C_mesh:
	{
		toAdd = new mesh(res, this);
		break;
	}
	case Component::Type::C_material:
	{
		if (HasComponent(Component::C_material) == false)
		{
			toAdd = new Material(res, this);
		}
		break;
	}
	case Component::Type::C_camera:
	{
		if (HasComponent(Component::Type::C_transform))
		{
			toAdd = new Camera(this);
		}
		break;
	}
	case Component::Type::C_Billboard:
	{
		if (HasComponent(Component::C_Billboard) == false)
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
			HasComponents[toAdd->GetType()] += 1;
			components.push_back(toAdd);
			App->m_goManager->components.insert(std::pair<Component::Type, Component*>(toAdd->GetType(), toAdd));
			if (toAdd->GetType() == Component::Type::C_mesh)
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

bool GameObject::HasComponent(Component::Type type)
{
	return (HasComponents[type] != 0);
}

uint GameObject::AmountOfComponent(Component::Type type)
{
	return HasComponents[type];
}

Transform * GameObject::GetTransform()
{
	return transform;
}

void GameObject::Delete()
{
	App->m_editor->UnselectGameObject(this);
	App->m_goManager->DeleteGameObject(this);
}

void GameObject::Save(pugi::xml_node& node)
{
	if (hiddenOnOutliner == false)
	{
		pugi::xml_node GO = node.append_child("GO");
		GO.append_attribute("Active") = active;
		GO.append_attribute("Static") = Static;
		GO.append_attribute("name") = name;
		if (parent != nullptr)
		{
			GO.append_attribute("UID") = uid;
			if (parent->parent != nullptr)
			{
				GO.append_attribute("parent") = parent->GetUID();
			}
			else
			{
				GO.append_attribute("parent") = 0;
			}
		}
		else
		{
			GO.append_attribute("UID") = 0;
			GO.append_attribute("parent") = "0";
		}

		for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
		{
			(*it)->Save(node);
		}
	}
}

void GameObject::RemoveComponent(Component * comp)
{
	std::vector<Component*>::iterator it = components.begin();
	for (; it != components.end(); it++)
	{
		if ((*it) == comp)
		{
			std::map<Component::Type, Component*>::iterator mapIt = App->m_goManager->components.find((*it)->GetType());
			for (; mapIt != App->m_goManager->components.end() && mapIt->first == (*it)->GetType(); mapIt++)
			{
				if (mapIt->second == comp)
				{
					App->m_goManager->components.erase(mapIt);
					break;
				}
			}
			HasComponents[(*it)->GetType()] -= 1;
			RELEASE(*it);
			components.erase(it);
			return;
		}
	}
}
