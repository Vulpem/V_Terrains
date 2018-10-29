#include "Globals.h"
#include "Application.h"
#include "PhysBody3D.h"
#include "Primitive.h"
#include "ModuleCamera3D.h"
#include "ModuleInput.h"
#include "ModuleGOmanager.h"

#include "Transform.h"

ModuleCamera3D::ModuleCamera3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
void ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	m_camSpeed = 200.f;
	m_camSprintMultiplier = 10.f;

	m_defaultCameraGO = App->m_goManager->CreateCamera("DefaultEditorCamera");
	m_defaultCamera = m_defaultCameraGO->GetComponent<Camera>().front();
	m_defaultCamera->SetFarPlane(50000.0);
	m_defaultCameraGO->HideFromOutliner();

	m_topView = App->m_goManager->CreateCamera("m_topView");
	m_topView->GetTransform()->SetLocalPos(0, 1000, 0);
	m_topView->GetTransform()->SetLocalRot(90, 0, 0);
	m_topView->GetTransform()->allowRotation = false;
	m_topView->GetComponent<Camera>().front()->SetFarPlane(50000.000);
	m_topView->GetComponent<Camera>().front()->SwitchViewType();
    m_topView->GetComponent<Camera>().front()->SetHorizontalFOV(50000);
	m_topView->HideFromOutliner();
}


// -----------------------------------------------------------------
UpdateStatus ModuleCamera3D::Update()
{
	if (Time.PlayMode == Play::Stop)
	{
		MoveWithKeys();
		// Mouse motion ----------------
		if (App->m_input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
		{
			int dx = -App->m_input->GetMouseXMotion();
			int dy = -App->m_input->GetMouseYMotion();
			if (dx != 0 || dy != 0)
			{
				float Sensitivity = 0.04;
				Transform* cam = GetMovingCamera()->object->GetTransform();

				float3 toLook = cam->GetGlobalPos();
				toLook += cam->Forward() * 10;

				toLook += (float)dy * Sensitivity *cam->Up();

				toLook += (float)dx * Sensitivity * cam->Left();

				LookAt(toLook);
			}
		}
	}
    if (m_followCamera)
    {
        math::float3 pos = m_defaultCameraGO->GetTransform()->GetGlobalPos();
        m_topView->GetTransform()->SetGlobalPos(pos.x, m_topView->GetTransform()->GetGlobalPos().y, pos.z);
    }

	return UpdateStatus::Continue;
}


// -----------------------------------------------------------------
void ModuleCamera3D::LookAt( const float3 &Spot)
{
	GetMovingCamera()->object->GetTransform()->LookAt(Spot);
}


// -----------------------------------------------------------------
void ModuleCamera3D::Move(const float3 &Movement)
{
	GetMovingCamera()->object->GetTransform()->SetGlobalPos(GetMovingCamera()->object->GetTransform()->GetGlobalPos() + Movement);
}

void ModuleCamera3D::SetPos(const float3 &Pos)
{
	GetMovingCamera()->object->GetTransform()->SetGlobalPos(Pos);
}

// -----------------------------------------------------------------

Camera * ModuleCamera3D::GetDefaultCam()
{
	return m_defaultCamera;
}

Camera * ModuleCamera3D::GetTopCam()
{
	return m_topView->GetComponent<Camera>().front();
}

Camera * ModuleCamera3D::GetRightCam()
{
	return m_rightView->GetComponent<Camera>().front();
}

Camera * ModuleCamera3D::GetFrontCam()
{
	return m_frontView->GetComponent<Camera>().front();
}

void ModuleCamera3D::SetCameraToDefault(Camera* toSet)
{
	SetCameraToCamera(m_defaultCameraGO, toSet);
}

void ModuleCamera3D::SetCameraToTop(Camera* toSet)
{
	SetCameraToCamera(m_topView, toSet);
}

void ModuleCamera3D::SetCameraToRight(Camera* toSet)
{
	SetCameraToCamera(m_rightView, toSet);
}

void ModuleCamera3D::SetCameraToFront(Camera* toSet)
{
	SetCameraToCamera(m_frontView, toSet);
}

void ModuleCamera3D::SetCameraToCamera(GameObject * setTo, Camera* toSet)
{
	if (setTo->HasComponent<Camera>() && setTo->GetTransform() != nullptr)
	{
		toSet->object->GetTransform()->SetLocalPos(setTo->GetTransform()->GetLocalPos());
		toSet->object->GetTransform()->SetLocalRot(setTo->GetTransform()->GetLocalRot());
		if (toSet->GetFrustum()->type != setTo->GetComponent<Camera>().front()->GetFrustum()->type)
		{
			toSet->SwitchViewType();
		}
	}
}


Camera * ModuleCamera3D::GetMovingCamera()
{
	Camera* cam = m_movingCamera;
	if (cam == nullptr)
	{
		cam = GetDefaultCam();
	}
	return cam;
}

void ModuleCamera3D::SetMovingCamera(Camera * cam)
{
	m_movingCamera = cam;
}

void ModuleCamera3D::MoveWithKeys()
{
	Camera* cam = GetMovingCamera();

	float speed = m_camSpeed;
	float3 lastCamPos = cam->object->GetTransform()->GetGlobalPos();
	float3 camPos = lastCamPos;
	if (App->m_input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
	{
		speed *= m_camSprintMultiplier;
	}

	int mouseWheel = App->m_input->GetMouseZ();
	if (mouseWheel != 0)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Forward() * speed * mouseWheel * 30 * Time.dt;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov - speed * mouseWheel * 30 * Time.dt);
		}
	}

	if (App->m_input->GetMouseButton(2))
	{
		lastCamPos += cam->object->GetTransform()->Left() * speed* Time.dt * App->m_input->GetMouseXMotion();
		lastCamPos += cam->object->GetTransform()->Up() * speed* Time.dt * App->m_input->GetMouseYMotion();
	}

	//Forward Backward
	//In Ortographic mode, moving the camera forward or backward is meaningless. Instead we'll change the FOV to change the zoom lvl
	if (App->m_input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Forward() * speed* Time.dt;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov - speed * 10.f * Time.dt);
		}
	}
	if (App->m_input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Backward() * speed* Time.dt;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov + speed * 10.f * Time.dt);
		}
	}

	//Right Left
	if (App->m_input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Left() * speed* Time.dt;
	}
	if (App->m_input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Right() * speed* Time.dt;
	}

	//Up Down
	if (App->m_input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Down() * speed* Time.dt;
	}
	if (App->m_input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Up() * speed* Time.dt;
	}

	if (lastCamPos.x != camPos.x || lastCamPos.y != camPos.y || lastCamPos.z != camPos.z)
	{
		cam->object->GetTransform()->SetGlobalPos(lastCamPos);
	}
}
