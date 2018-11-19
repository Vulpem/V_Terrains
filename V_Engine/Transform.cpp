#include "Transform.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

#include "GameObject.h"
#include "Camera.h"
#include "Application.h"
#include "Globals.h"
#include "ModuleGOmanager.h"

#include "imGUI\imgui.h"
#include "ViewPort.h"


Transform::Transform(Gameobject* linkedTo)
	: m_gameObject(linkedTo)
	, m_localPosition(0,0,0)
	, m_localScale(1,1,1)
	, m_localRotation(math::Quat::identity)
{ }

Transform::~Transform()
{
	SetParent(nullptr);

	if (IsStatic() == false)
	{
		if (App->m_goManager->m_dynamicGO.empty() == false)
		{
			for (std::vector<Gameobject*>::iterator it = App->m_goManager->m_dynamicGO.begin(); it != App->m_goManager->m_dynamicGO.end(); it++)
			{
				if ((*it) == GetGameobject())
				{
					App->m_goManager->m_dynamicGO.erase(it);
					break;
				}
			}
		}
	}
	else
	{
		App->m_goManager->m_quadTree.Remove(GetGameobject());
	}
}

void Transform::SaveSpecifics(pugi::xml_node& myNode) const
{
	myNode.append_attribute("AllowRotation") = m_allowRotation;
	myNode.append_attribute("m_static") = m_static;
	pugi::xml_node node = myNode.append_child("LocalRotation");
	node.append_attribute("x") = m_localRotation.x;
	node.append_attribute("y") = m_localRotation.y;
	node.append_attribute("z") = m_localRotation.z;
	node.append_attribute("w") = m_localRotation.w;

	node = myNode.append_child("LocalPosition");
	node.append_attribute("x") = m_localPosition.x;
	node.append_attribute("y") = m_localPosition.y;
	node.append_attribute("z") = m_localPosition.z;

	node = myNode.append_child("LocalScale");
	node.append_attribute("x") = m_localScale.x;
	node.append_attribute("y") = m_localScale.y;
	node.append_attribute("z") = m_localScale.z;
}

void Transform::LoadSpecifics(pugi::xml_node & myNode)
{
	m_allowRotation = myNode.attribute("AllowRotation").as_bool();
	bool setToStatic = myNode.attribute("m_static").as_bool();
	//TODO set to static
	float tmp[4];

	pugi::xml_node rot = myNode.child("LocalRotation");
	tmp[0] = rot.attribute("x").as_float();
	tmp[1] = rot.attribute("y").as_float();
	tmp[2] = rot.attribute("z").as_float();
	tmp[3] = rot.attribute("w").as_float();
	SetLocalRot(tmp[0], tmp[1], tmp[2], tmp[3]);

	pugi::xml_node pos = myNode.child("LocalPosition");
	tmp[0] = pos.attribute("x").as_float();
	tmp[1] = pos.attribute("y").as_float();
	tmp[2] = pos.attribute("z").as_float();
	SetLocalPos(tmp[0], tmp[1], tmp[2]);

	pugi::xml_node scal = myNode.child("LocalScale");
	tmp[0] = scal.attribute("x").as_float();
	tmp[1] = scal.attribute("y").as_float();
	tmp[2] = scal.attribute("z").as_float();
	SetLocalScale(tmp[0], tmp[1], tmp[2]);

}

void Transform::Draw(const ViewPort & port) const
{
	if (Time.PlayMode != PlayMode::Play)
	{
		float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f);
		if (GetGameobject()->IsSelected())
		{
			if (GetParent()->GetGameobject()->IsSelected())
			{
				color = float4(0, 0.5f, 0.5f, 1);
			}
			else {
				color = float4(0, 0.8f, 0.8f, 1);
			}
		}
		App->m_renderer3D->DrawLocator(GetGlobalTransform(), color);

		std::vector<Transform*> childs = GetChilds();
		for (auto child : childs)
		{
			math::float3 childPos(child->GetGlobalPos());
			App->m_renderer3D->DrawLine(GetGlobalPos(), childPos, color);
		}
	}

	if (port.m_renderBoundingBoxes)
	{
		m_gameObject->DrawAABB();
		m_gameObject->DrawOBB();
	}
}

void Transform::EditorContent()
{
	ImGui::Text("m_static: ");
	ImGui::SameLine();
	bool isStatic = m_static;
	ImGui::Checkbox("##isObjectStatic", &isStatic);
	if (isStatic != m_static && App->m_goManager->m_setting == nullptr)
	{
		if (GetChilds().empty() == true)
		{
			App->m_goManager->SetStatic(isStatic, GetGameobject());
		}
		else
		{
			App->m_goManager->m_setting = GetGameobject();
			App->m_goManager->m_settingStatic = isStatic;
		}
	}

	float tmp[3];
	tmp[0] = m_localPosition.x;
	tmp[1] = m_localPosition.y;
	tmp[2] = m_localPosition.z;
	if (ImGui::DragFloat3("Position", tmp, 1.0f))
	{
		SetLocalPos(tmp[0], tmp[1], tmp[2]);
	}

	//math::float3 rot = GetLocalRot();
	tmp[0] = m_editorRot.x;
	tmp[1] = m_editorRot.y;
	tmp[2] = m_editorRot.z;
	for (int n = 0; n < 3; n++)
	{
		tmp[n] = tmp[n] - FloorInt(tmp[n] / 360) * 360;
	}
	if (ImGui::DragFloat3("Rotation", tmp, 1.0f))
	{
		SetLocalRot(tmp[0], tmp[1], tmp[2]);
		m_editorRot.x = tmp[0];
		m_editorRot.y = tmp[1];
		m_editorRot.z = tmp[2];
		m_editorGlobalRot = GetGlobalRot();
	}

	tmp[0] = m_localScale.x;
	tmp[1] = m_localScale.y;
	tmp[2] = m_localScale.z;
	if (ImGui::DragFloat3("Scale", tmp, 0.01f, 0.1f))
	{
		SetLocalScale(tmp[0], tmp[1], tmp[2]);
	}
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Text("Global transformations");

	float3 pos = GetGlobalPos();
	if (ImGui::DragFloat3("Global Pos##G_Pos", pos.ptr(), 1.0f))
	{
		SetGlobalPos(pos.x, pos.y, pos.z);
	}

	tmp[0] = m_editorGlobalRot.x;
	tmp[1] = m_editorGlobalRot.y;
	tmp[2] = m_editorGlobalRot.z;
	if (ImGui::DragFloat3("Global Rot##G_Rot", tmp, 1.0f))
	{
		SetGlobalRot(tmp[0], tmp[1], tmp[2]);
		m_editorGlobalRot.x = tmp[0];
		m_editorGlobalRot.y = tmp[1];
		m_editorGlobalRot.z = tmp[2];
		m_editorRot = GetLocalRot();
	}

	float3 scal = GetGlobalScale();
	ImGui::Text("%.2f, %.2f, %.2f  Global scale", scal.x, scal.y, scal.z);

}

math::float4x4 Transform::GetLocalTransformMatrix() const
{
		math::float4x4 transform = math::float4x4::FromTRS(m_localPosition, m_localRotation.ToFloat3x3(), m_localScale);
		transform.Transpose();
		return transform;
}

void Transform::UpdateGlobalTransform()
{
	if (m_gameObject->GetTransform()->GetParent() != nullptr)
	{
		Transform* m_parent = m_gameObject->GetTransform()->GetParent();
		
		m_globalTransform = GetLocalTransformMatrix() * m_parent->GetGlobalTransform();
	}
	else
	{
		m_globalTransform = GetLocalTransformMatrix();
	}
}

math::float4x4 Transform::GetGlobalTransform() const
{
	return m_globalTransform;
}

void Transform::UpdateEditorValues()
{
	m_editorRot = GetLocalRot();
	m_editorGlobalRot = GetGlobalRot();
}

void Transform::SetLocalPos(float x, float y, float z)
{
	if (IsStatic() == false)
	{
		m_localPosition.x = x;
		m_localPosition.y = y;
		m_localPosition.z = z;

		m_gameObject->PositionChanged();

		if (m_gameObject->HasComponent<Camera>())
		{
			std::vector<Camera*> cams;
			m_gameObject->GetComponents<Camera>(cams);
			for (auto it : cams)
			{
				it->UpdatePos();
				it++;
			}
		}
	}
}

void Transform::SetLocalPos(float3 pos)
{
	SetLocalPos(pos.x, pos.y, pos.z);
}

math::float3 Transform::GetLocalPos() const
{
	return m_localPosition;
}

void Transform::SetGlobalPos(float x, float y, float z)
{
	if (IsStatic() == false)
	{
		if (m_gameObject->GetTransform()->GetParent() != nullptr)
		{
			//TODO: clean transform SetGlobalPos
			Transform* parentTrans = m_gameObject->GetTransform()->GetParent();

			float4x4 myGlobal = (float4x4::FromTRS(float3(x, y, z), GetGlobalRotQuat(), GetGlobalScale()));
			float4x4 parentGlobal = parentTrans->GetGlobalTransform();

			float4x4 localMat = myGlobal.Transposed() * parentGlobal.Inverted();
			localMat.Transpose();

			SetLocalPos(localMat.TranslatePart().x, localMat.TranslatePart().y, localMat.TranslatePart().z);
		}
		else
		{
			SetLocalPos(x, y, z);
		}
	}
}

void Transform::SetGlobalPos(float3 pos)
{
	SetGlobalPos(pos.x, pos.y, pos.z);
}

math::float3 Transform::GetGlobalPos() const
{
	math::float4x4 p = m_globalTransform.Transposed();
	return p.TranslatePart();
}

void Transform::Translate(float x, float y, float z)
{
	Translate(float3(x, y, z));
}

void Transform::Translate(float3 m)
{
	SetGlobalPos(GetGlobalPos() + m);
}

void Transform::SetLocalRot(float x, float y, float z)
{
	if (IsStatic() == false && m_allowRotation)
	{
		x = x - FloorInt(x / 360) * 360;
		y = y - FloorInt(y / 360) * 360;
		z = z - FloorInt(z / 360) * 360;
		x *= DEGTORAD;
		y *= DEGTORAD;
		z *= DEGTORAD;

		m_localRotation = math::Quat::FromEulerXYZ(x, y, z);

		m_gameObject->PositionChanged();
		UpdateEditorValues();
	}
}

void Transform::SetLocalRot(float3 rot)
{
	SetLocalRot(rot.x, rot.y, rot.z);
}

void Transform::SetLocalRot(float x, float y, float z, float w)
{
	if (IsStatic() == false && m_allowRotation)
	{
		m_localRotation.Set(x, y, z, w);

		m_gameObject->PositionChanged();
	}
}

math::float3 Transform::GetLocalRot() const
{
	math::float3 ret = m_localRotation.ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;

	return ret;
}

Quat Transform::GetLocalRotQuat() const
{
	return m_localRotation;
}

void Transform::SetGlobalRot(float x, float y, float z)
{
	if (IsStatic() == false && m_allowRotation)
	{
		if (m_gameObject->GetTransform()->GetParent() != nullptr)
		{
			x *= DEGTORAD;
			y *= DEGTORAD;
			z *= DEGTORAD;

			//Quat::from

			Transform* parentTrans = m_gameObject->GetTransform()->GetParent();
			const Quat local = Quat::FromEulerXYZ(x, y, z) * parentTrans->GetGlobalRotQuat().Conjugated();
			SetLocalRot(local.x, local.y, local.z, local.w);
		}
		else
		{
			SetLocalRot(x, y, z);
		}
	}
}

void Transform::SetGlobalRot(float3 rotation)
{
	SetGlobalRot(rotation.x, rotation.y, rotation.z);
}

void Transform::RotateLocal(float3 rotation)
{
	if(rotation.x != 0.f || rotation.y != 0.f || rotation.z != 0.f)
	{
		m_localRotation = m_localRotation * Quat::FromEulerXYZ(rotation.x * DEGTORAD, rotation.y* DEGTORAD, rotation.z* DEGTORAD);
		m_gameObject->PositionChanged();
		UpdateEditorValues();
	}
}

math::Quat Transform::GetGlobalRotQuat() const
{
	float3 pos;
	float3 scal;
	Quat ret;
	m_globalTransform.Transposed().Decompose(pos, ret, scal);
	return ret;
}

math::float3 Transform::GetGlobalRot() const
{
	math::float3 ret = m_globalTransform.Transposed().ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;
	return ret;
}

void Transform::SetLocalScale(float x, float y, float z)
{
	if (IsStatic() == false)
	{
		if (x != 0 && y != 0 && z != 0)
		{
			m_localScale.Set(x, y, z);

			m_gameObject->PositionChanged();
		}
	}
}

math::float3 Transform::GetLocalScale() const
{
	return m_localScale;
}

math::float3 Transform::GetGlobalScale() const
{
	return m_globalTransform.ExtractScale();
}

void Transform::LookAt(const float3 & Spot, float3 worldUp)
{
	if (IsStatic() == false)
	{
		float4x4 tmp = float4x4::LookAt(GetGlobalPos(), Spot, float3(0,0,1), float3(0,1,0), worldUp);
		SetGlobalRot(tmp.ToEulerXYZ() * RADTODEG);
		UpdateEditorValues();
	}
}



float3 Transform::Up() const
{
	return m_globalTransform.Transposed().WorldY().Normalized();
}

float3 Transform::Down() const
{
	return -Up();
}

float3 Transform::Left() const
{
	return m_globalTransform.Transposed().WorldX().Normalized();
}

float3 Transform::Right() const
{
	return -Left();
}

float3 Transform::Forward() const
{
	return m_globalTransform.Transposed().WorldZ().Normalized();
}

float3 Transform::Backward() const
{
	return -Forward();
}

float3 Transform::WorldUp()
{
	return float3(0,1,0);
}

float3 Transform::WorldDown()
{
	return float3(0, -1, 0);
}

float3 Transform::WorldLeft()
{
	return float3(1, 0, 0);
}

float3 Transform::WorldRight()
{
	return float3(-1, 0, 0);
}

float3 Transform::WorldForward()
{
	return float3(0, 0,1);
}

float3 Transform::WorldBackward()
{
	return float3(0, 0,-1);
}

Gameobject * Transform::GetGameobject() const
{
	return m_gameObject;
}

void Transform::SetParent(Transform * newParent)
{
	if (m_parent != nullptr)
	{
		m_parent->m_childs.erase(std::find(m_parent->m_childs.begin(), m_parent->m_childs.end(), this));
		m_parent = nullptr;
	}
	if (newParent != nullptr)
	{
		newParent->m_childs.push_back(this);
		m_parent = newParent;
	}
}

Transform * Transform::GetParent() const
{
	return m_parent;
}

void Transform::AddChild(Transform * newChild)
{
	newChild->SetParent(this);
}

std::vector<Transform*> Transform::GetChilds() const
{
	return m_childs;
}
