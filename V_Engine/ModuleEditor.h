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

	bool Init() override;

	void Start() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;

	void OnPlay() override;
	void OnStop() override;

	void CleanUp() override;

	void Render(const ViewPort& port) const override;

	void OnScreenResize(int width, int heigth) override;

	void HandleInput(SDL_Event* event);

    void SwitchViewPorts();

	void UnselectGameObject(GameObject* go);
private:

	void MenuBar();
	void Editor();
    void ViewPortUI(const ViewPort & port) const;
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