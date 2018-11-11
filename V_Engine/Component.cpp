#include "Component.h"
#include "GameObject.h"

#include "Application.h"
#include "ModuleResourceManager.h"

#include "ImGui\imgui.h"

Component::Component(GameObject* linkedTo, ComponentType type): m_name("Empty component"), m_type(type)
{
	m_gameObject = linkedTo;
	m_uid = GenerateUUID();
}

Component::~Component()
{
}

void Component::Enable()
{
	if (m_enabled == false)
	{
		OnEnable();
		m_enabled = true;
	}
}

void Component::Disable()
{ 
	if (m_enabled == true)
	{
		OnDisable();
		m_enabled = false;
	}
}


void Component::DrawOnEditor()
{
	bool active = m_enabled;
	char _id[256];
	sprintf(_id, "##checkbox%llu", m_uid);
	ImGui::Checkbox(_id, &active);

	if (active != m_enabled)
	{
		if (active)
		{
			Enable();
		}
		else
		{
			Disable();
		}
	}


	ImGui::SameLine(ImGui::GetWindowWidth() - 50);
	sprintf(_id, "X##RemoveComponent%llu", m_uid);
	if (ImGui::Button(_id, ImVec2(25, 20)))
	{
		Delete();
	}
	ImGui::SameLine(30);

	bool open = ImGui::CollapsingHeader(m_name.data());

	if (open && m_enabled)
	{
		if (MissingComponent() == false)
		{
			EditorContent();
		}
		else
		{
			ImGui::Text("Something went terribly wrong.");
			ImGui::Text("¡This component is missing its resource!");
		}
	}
}

void Component::Save(pugi::xml_node& myNode)
{
	pugi::xml_node node = myNode.append_child("General");
	char tmpName[256];
	strcpy(tmpName, m_name.data());
	char* it = tmpName;
	for (int n = 0; n < 255; n++)
	{
		if (it[0] == '#' && it[1] == '#')
		{
			it[0] = '\0';
			break;
		}
		it++;
	}

	node.append_attribute("name") = tmpName;
	node.append_attribute("UID") = m_uid;
	node.append_attribute("type") = static_cast<int>(m_type);
	node.append_attribute("GO") = m_gameObject->GetUID();
	node.append_attribute("enabled") = IsEnabled();

	SaveSpecifics(myNode.append_child("Specific"));
}

GameObject * Component::GetOwner() const
{
	return m_gameObject;
}

bool Component::MarkedForDeletion() const
{
	return m_toDelete;
}

void Component::Delete()
{
	m_toDelete = true;
}

bool Component::TryDeleteNow()
{
	if (m_toDelete)
	{
		m_gameObject->RemoveComponent(this);
		return true;
	}
	return false;
}
