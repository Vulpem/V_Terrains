#include "Camera.h"

#include "GameObject.h"
#include "Transform.h"

#include "imGUI\imgui.h"

#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"


//------------------------- Camera --------------------------------------------------------------------------------

Camera::Camera(GameObject* linkedTo):Component(linkedTo, ComponentType::camera)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Camera##%i", m_uid);
	m_name = tmp;

	m_positionOffset = float3::zero;
	if (m_gameObject)
	{
		AABB goAABB = m_gameObject->GetAABB();
		if (goAABB.IsFinite())
		{
			m_positionOffset = goAABB.CenterPoint() - m_gameObject->GetTransform()->GetGlobalPos();
		}
	}
	m_frustum.nearPlaneDistance = 4;
	m_frustum.farPlaneDistance = 50000.0;
	m_frustum.type = FrustumType::PerspectiveFrustum;

	float2 screenSize = App->m_window->GetWindowSize();
	m_aspectRatio = (screenSize.x / screenSize.y);

	SetHorizontalFOV(60*DEGTORAD);
	UpdateCamMatrix();
}

Camera::~Camera()
{
	if (App->m_camera->GetMovingCamera() == this)
	{
		App->m_camera->SetMovingCamera();
	}
}

void Camera::PreUpdate()
{
	m_frustum.Update();
}

float3 Camera::GetPosition()
{
    return m_gameObject->GetTransform()->GetGlobalPos();
}

void Camera::Draw(const ViewPort & port)
{
	if (m_gameObject->GetTransform()->m_hiddenOnOutliner == false)
	{
		DrawFrustum();
	}
}

void Camera::PositionChanged()
{
	UpdateCamMatrix();
}

void Camera::UpdateCamMatrix()
{
	AABB goAABB = m_gameObject->GetAABB();
	if (goAABB.IsFinite())
	{
		m_positionOffset = goAABB.CenterPoint() - m_gameObject->GetTransform()->GetGlobalPos();
	}
	UpdateOrientation();
	UpdatePos();
}

void Camera::UpdatePos()
{
	m_frustum.pos = m_gameObject->GetTransform()->GetGlobalPos() + m_positionOffset;
}

void Camera::UpdateOrientation()
{
	float3 rotation = m_gameObject->GetTransform()->GetGlobalRot();
	rotation *= DEGTORAD;
	float4x4 toSend = float4x4::FromEulerXYZ(rotation.x, rotation.y, rotation.z);
	m_frustum.SetWorldMatrix(toSend.Float3x4Part());
	m_frustum.front = m_gameObject->GetTransform()->GetGlobalTransform().Transposed().WorldZ().Normalized();
}

FrustumCollision Camera::Collides(AABB boundingBox)
{
	float3 points[8];
	boundingBox.GetCornerPoints(points);

	for (int p = 0; p < 6; p++)
	{
		for (int n = 0; n < 8; n++)
		{
			if (m_frustum.GetPlane(p).IsOnPositiveSide(points[n]) == false)
			{
				break;
			}
			if (n == 7)
			{
				return FrustumCollision::outside;
			}
		}
	}
	return FrustumCollision::contains;
}

FrustumCollision Camera::Collides(float3 point)
{
	Plane planes[6];
	m_frustum.GetPlanes(planes);

	for (int n = 0; n < 6; n++)
	{
		if (planes[n].IsOnPositiveSide(point) == true)
		{
			return FrustumCollision::outside;
		}
	}
	return FrustumCollision::contains;
}

math::FrustumType Camera::SwitchViewType()
{
	if (m_frustum.type == FrustumType::PerspectiveFrustum)
	{
		m_otherFOV.x = m_frustum.horizontalFov;
		m_otherFOV.y = m_frustum.verticalFov;
		m_frustum.type = FrustumType::OrthographicFrustum;
		SetHorizontalFOV(m_frustum.horizontalFov * 100.0f);
		return FrustumType::OrthographicFrustum;
	}
	m_frustum.type = FrustumType::PerspectiveFrustum;
	m_aspectRatio = m_otherFOV.x / m_otherFOV.y;
	SetHorizontalFOV(m_otherFOV.x);
	return FrustumType::PerspectiveFrustum;
}

void Camera::EditorContent()
{
	ImGui::Checkbox("Culling", &m_forceDebugCulling);
	bool persp = true;
	bool ortho = false;
	if (m_frustum.type == FrustumType::OrthographicFrustum)
	{
		persp = false;
		ortho = true;
	}
	ImGui::Checkbox("Perspective", &persp);
	ImGui::SameLine();
	ImGui::Checkbox("Orthographic", &ortho);
	if (persp == ortho)
	{
		SwitchViewType();
	}


	ImGui::Text("Position:");
	ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", m_frustum.pos.x, m_frustum.pos.y, m_frustum.pos.z);
	ImGui::Text("Position Offset");
	if (ImGui::DragFloat3("##posOffset", m_positionOffset.ptr(), 0.1f))
	{
		UpdatePos();
	}

	ImGui::Text("AspectRatio");
	if (ImGui::DragFloat("##Aspect_ratio", &m_aspectRatio, 0.1f, 0.1f, 5.0f))
	{
		SetHorizontalFOV(m_frustum.horizontalFov);
	}
	float tmp = m_frustum.horizontalFov * RADTODEG;
	float maxFOV = 180;
	if (m_frustum.type == FrustumType::OrthographicFrustum)
	{
		maxFOV = floatMax;
	}
	if (ImGui::DragFloat("Horizontal FOV", &tmp, 1.0f, 1.0f, maxFOV))
	{
		SetHorizontalFOV(tmp * DEGTORAD);
	}
	tmp = m_frustum.verticalFov * RADTODEG;
	if (ImGui::DragFloat("Vertical FOV", &tmp, 1.0f, 1.0f, maxFOV))
	{
		SetHorizontalFOV(tmp * m_aspectRatio * DEGTORAD);
	}

	if (ImGui::DragFloat("NearPlane", &m_frustum.nearPlaneDistance, 0.1f, 0.1f, m_frustum.farPlaneDistance - 1.0f))
	{
		SetHorizontalFOV(m_frustum.horizontalFov);
	}
	if (ImGui::DragFloat("FarPlane", &m_frustum.farPlaneDistance, 1.0f, m_frustum.nearPlaneDistance + 1.0f, 400000.0f, "%.3f", 3));
	{
		SetHorizontalFOV(m_frustum.horizontalFov);
	}
}

void Camera::SaveSpecifics(pugi::xml_node& myNode)
{
	pugi::xml_node frust = myNode.append_child("Frustum");
	frust.append_attribute("FarPlaneDistance") = m_frustum.farPlaneDistance;
	frust.append_attribute("NearPlaneDistance") = m_frustum.nearPlaneDistance;
	frust.append_attribute("HorizontalFOV") = m_frustum.horizontalFov;
	frust.append_attribute("AspectRatio") = m_aspectRatio;
	frust.append_attribute("FrustumType") =	m_frustum.type;
	frust.append_attribute("FrontX") = m_frustum.front.x;
	frust.append_attribute("FrontY") = m_frustum.front.y;
	frust.append_attribute("FrontZ") = m_frustum.front.z;
	frust.append_attribute("UpX") = m_frustum.up.x;
	frust.append_attribute("UpY") = m_frustum.up.y;
	frust.append_attribute("UpZ") = m_frustum.up.z;
	frust.append_attribute("HasCulling") = m_forceDebugCulling;
	frust.append_attribute("otherFOV") = m_otherFOV.x;
}

void Camera::LoadSpecifics(pugi::xml_node & myNode)
{
	pugi::xml_node frust = myNode.child("Frustum");

	m_frustum.farPlaneDistance = frust.attribute("FarPlaneDistance").as_float();
	m_frustum.nearPlaneDistance = frust.attribute("NearPlaneDistance").as_float();
	m_frustum.type = (FrustumType)frust.attribute("FrustumType").as_int();
	
	m_aspectRatio = frust.attribute("AspectRatio").as_float();
	SetHorizontalFOV(frust.attribute("HorizontalFOV").as_float());

	m_frustum.front.x = frust.attribute("FrontX").as_float();
	m_frustum.front.y = frust.attribute("FrontY").as_float();
	m_frustum.front.z = frust.attribute("FrontZ").as_float();

	m_frustum.up.x = frust.attribute("UpX").as_float();
	m_frustum.up.y = frust.attribute("UpY").as_float();
	m_frustum.up.z = frust.attribute("UpZ").as_float();

	m_forceDebugCulling = frust.attribute("HasCulling").as_bool();
	m_otherFOV.x = frust.attribute("otherFOV").as_float();
	m_otherFOV.y = m_otherFOV.x * m_aspectRatio;
}

void Camera::DrawFrustum()
{
	if (GetOwner()->IsSelected())
	{
		float3 corners[8];
		m_frustum.GetCornerPoints(corners);
		App->m_renderer3D->DrawBox(corners, float4(0.49f, 0.85f, 1.0f, 1.0f));
	}
}

void Camera::SetHorizontalFOV(float horizontalFOV)
{
	m_frustum.horizontalFov = horizontalFOV;
	m_frustum.verticalFov = horizontalFOV / m_aspectRatio;
}

void Camera::SetFarPlane(float farPlaneDistance)
{
	if (farPlaneDistance > m_frustum.nearPlaneDistance)
	{
		m_frustum.farPlaneDistance = farPlaneDistance;
	}
}

void Camera::SetNearPlane(float nearPlaneDistance)
{
	if (nearPlaneDistance < m_frustum.farPlaneDistance)
	{
		m_frustum.nearPlaneDistance = nearPlaneDistance;
	}
}

float4x4 Camera::GetViewMatrix()
{
	return float4x4(m_frustum.ViewMatrix()).Transposed();
}

float4x4 Camera::GetProjectionMatrix()
{
	return m_frustum.ProjectionMatrix().Transposed();
}
