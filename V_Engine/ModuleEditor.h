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
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();

	bool CleanUp();

	void Render(const viewPort& port);

	void OnScreenResize(int width, int heigth);

	void HandleInput(SDL_Event* event);

    void SwitchViewPorts();
private:

	update_status MenuBar();
	void Editor();
    void ViewPortUI(const viewPort & port);
	void AttributeWindow();

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