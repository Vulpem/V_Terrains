#include "Billboard.h"

#include "GameObject.h"
#include "Transform.h"
#include "Application.h"

#include "imGUI\imgui.h"

Billboard::Billboard(GameObject* linkedTo) : Component(linkedTo, ComponentType::billboard)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Billboard##%i", m_uid);
	m_name = tmp;
}

void Billboard::UpdateNow(const float3& point, const float3& _up)
{
	if (_up.IsZero() == false)
	{
		Transform& trans = m_gameObject->GetTransform();
		float3 front = point - trans.GetGlobalPos();

		float4x4 tmp = float4x4::LookAt(m_localForward.Normalized(), front, m_localUp.Normalized(), _up);
		trans.SetGlobalRot(tmp.ToEulerXYZ() * RADTODEG);
	}
	else
	{
		m_gameObject->GetTransform().LookAt(point);
	}
}

void Billboard::EditorContent()
{
	ImGui::Text("Local Up");
	ImGui::DragFloat3("##DragLocalUp", m_localUp.ptr(), 0.05f, -1, 1);
	ImGui::Text("Local Forward");
	ImGui::DragFloat3("##DragLocalFront", m_localForward.ptr(), 0.05f, -1, 1);
}

void Billboard::SaveSpecifics(pugi::xml_node& myNode)
{
}

void Billboard::LoadSpecifics(pugi::xml_node & myNode)
{
}