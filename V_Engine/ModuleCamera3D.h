#ifndef __CAMERA_3D__
#define __CAMERA_3D__

#include "Module.h"
#include "Globals.h"
#include "Camera.h"


class ModuleCamera3D : public Module
{
public:
	ModuleCamera3D(Application* app, bool start_enabled = true);
	~ModuleCamera3D();

	void Start() override;
	UpdateStatus Update() override;

	void LookAt(const float3 &Spot);
	void Move(const float3 &Movement);
	void SetPos(const float3 &Pos);

	Camera* GetDefaultCam();
	Camera* GetTopCam();
	Camera* GetRightCam();
	Camera* GetFrontCam();

	void SetCameraToDefault(Camera* toSet);
	void SetCameraToTop(Camera* toSet);
	void SetCameraToRight(Camera* toSet);
	void SetCameraToFront(Camera* toSet);
	void SetCameraToCamera(GameObject* setTo, Camera* toSet);

	Camera* GetMovingCamera();
	void SetMovingCamera(Camera* cam = nullptr);

public:
	float m_camSpeed = 30.0f;
	float m_camSprintMultiplier = 3.0f;
	bool m_followCamera = false;

private:
	void MoveWithKeys();

private:
	Camera* m_movingCamera = nullptr;
	GameObject* m_defaultCameraGO = nullptr;
	Camera* m_defaultCamera = nullptr;

	GameObject* m_topView = nullptr;
	GameObject* m_rightView = nullptr;
	GameObject* m_frontView = nullptr;
public:
};

#endif