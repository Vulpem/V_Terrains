#include "Transform.h"

#include "GameObject.h"

#include "Camera.h"

#include "imGUI\imgui.h"
#include "ViewPort.h"


Transform::Transform(GameObject* linkedTo)
	: m_gameObject(linkedTo)
	, m_localPosition(0,0,0)
	, m_localScale(1,1,1)
	, m_localRotation(math::Quat::identity)
{ }

Transform::~Transform()
{
}

void Transform::SaveSpecifics(pugi::xml_node& myNode)
{
	myNode.append_attribute("AllowRotation") = m_allowRotation;
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

void Transform::Draw(const ViewPort & port)
{
	m_gameObject->DrawLocator();

	if (port.m_renderBoundingBoxes)
	{
		m_gameObject->DrawAABB();
		m_gameObject->DrawOBB();
	}
}

void Transform::EditorContent()
{
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
		while (tmp[n] >= 360)
		{
			tmp[n] -= 360;
		}
		while (tmp[n] < 0)
		{
			tmp[n] += 360;
		}
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
	ImGui::Text("%.2f, %.2f, %.2f", scal.x, scal.y, scal.z);

}

math::float4x4 Transform::GetLocalTransformMatrix()
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

math::float4x4 Transform::GetGlobalTransform()
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
	if (m_gameObject->IsStatic() == false)
	{
		m_localPosition.x = x;
		m_localPosition.y = y;
		m_localPosition.z = z;

		m_gameObject->UpdateTransformMatrix();

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

math::float3 Transform::GetLocalPos()
{
	return m_localPosition;
}

void Transform::SetGlobalPos(float x, float y, float z)
{
	if (m_gameObject->IsStatic() == false)
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

math::float3 Transform::GetGlobalPos()
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
	if (m_gameObject->IsStatic() == false && m_allowRotation)
	{
		while (x < 0) { x += 360; }
		while (y < 0) { y += 360; }
		while (z < 0) { z += 360; }

		x *= DEGTORAD;
		y *= DEGTORAD;
		z *= DEGTORAD;

		m_localRotation = math::Quat::FromEulerXYZ(x, y, z);

		m_gameObject->UpdateTransformMatrix();
		UpdateEditorValues();
	}
}

void Transform::SetLocalRot(float3 rot)
{
	SetLocalRot(rot.x, rot.y, rot.z);
}

void Transform::SetLocalRot(float x, float y, float z, float w)
{
	if (m_gameObject->IsStatic() == false && m_allowRotation)
	{
		m_localRotation.Set(x, y, z, w);

		m_gameObject->UpdateTransformMatrix();
	}
}

math::float3 Transform::GetLocalRot()
{
	math::float3 ret = m_localRotation.ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;

	return ret;
}

Quat Transform::GetLocalRotQuat()
{
	return m_localRotation;
}

void Transform::SetGlobalRot(float x, float y, float z)
{
	if (m_gameObject->IsStatic() == false && m_allowRotation)
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
		m_gameObject->UpdateTransformMatrix();
		UpdateEditorValues();
	}
}

math::Quat Transform::GetGlobalRotQuat()
{
	float3 pos;
	float3 scal;
	Quat ret;
	m_globalTransform.Transposed().Decompose(pos, ret, scal);
	return ret;
}

math::float3 Transform::GetGlobalRot()
{
	math::float3 ret = m_globalTransform.Transposed().ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;
	return ret;
}

void Transform::SetLocalScale(float x, float y, float z)
{
	if (m_gameObject->IsStatic() == false)
	{
		if (x != 0 && y != 0 && z != 0)
		{
			m_localScale.Set(x, y, z);

			m_gameObject->UpdateTransformMatrix();
		}
	}
}

math::float3 Transform::GetLocalScale()
{
	return m_localScale;
}

math::float3 Transform::GetGlobalScale()
{
	return m_globalTransform.ExtractScale();
}

void Transform::LookAt(const float3 & Spot, float3 worldUp)
{
	if (m_gameObject->IsStatic() == false)
	{
		float4x4 tmp = float4x4::LookAt(GetGlobalPos(), Spot, float3(0,0,1), float3(0,1,0), worldUp);
		SetGlobalRot(tmp.ToEulerXYZ() * RADTODEG);
		UpdateEditorValues();
	}
}



float3 Transform::Up()
{
	return m_globalTransform.Transposed().WorldY().Normalized();
}

float3 Transform::Down()
{
	return -Up();
}

float3 Transform::Left()
{
	return m_globalTransform.Transposed().WorldX().Normalized();
}

float3 Transform::Right()
{
	return -Left();
}

float3 Transform::Forward()
{
	return m_globalTransform.Transposed().WorldZ().Normalized();
}

float3 Transform::Backward()
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

GameObject * Transform::GetGameobject()
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

Transform * Transform::GetParent()
{
	return m_parent;
}

void Transform::AddChild(Transform * newChild)
{
	newChild->SetParent(this);
}

std::vector<Transform*> Transform::GetChilds()
{
	return m_childs;
}
