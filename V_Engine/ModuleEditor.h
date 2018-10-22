#ifndef __MODULE_UI__
#define __MODULE_UI__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include "ViewPort.h"

#include "ImGui\imgui.h"

class GameObject;

class ModuleEditor : public Module
{
public:
	
	ModuleEditor(Application* app, bool start_enabled = true);
	~ModuleEditor();

	bool Init();

	bool Start();
	UpdateStatus PreUpdate();
	UpdateStatus Update();
	UpdateStatus PostUpdate();

	void OnPlay() override;
	void OnStop() override;

	bool CleanUp();

	void Render(const ViewPort& port);

	void OnScreenResize(int width, int heigth);

	void HandleInput(SDL_Event* event);

    void SwitchViewPorts();

	void UnselectGameObject(GameObject* go);
private:

	UpdateStatus MenuBar();
	void Editor();
    void ViewPortUI(const ViewPort & port);
	void AttributeWindow();
	void Outliner();

	void SceneTreeGameObject(GameObject* node);

	void SelectByViewPort();
	void SelectGameObject(GameObject* node);
public:
	bool multipleViews = false;

	bool IsOpenTestWindow = false;

	bool showPlane = true;
	bool renderNormals = false;
    std::string m_shaderResult;

private:
	int screenW = 0;
	int screenH = 0;

	float2 viewPortMin;
	float2 viewPortMax;
	uint singleViewPort = 0;
	uint multipleViewPorts[2] = { 0,0 };
	uint fullScreenViewPort = 0;

	GameObject* selectedGameObject = nullptr;
	float selectedPos[3] = { 0,0,0 };
	float selectedScale[3] = { 1,1,1 };
	float selectedEuler[3] = { 0,0,0 };

	LineSegment selectRay;
	float3 out_normal;
	float3 out_pos;
public:
};

#endif