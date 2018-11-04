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
	void CleanUp() override;
	void Render(const ViewPort& port) const override;

	void OnPlay() override;
	void OnStop() override;
	void OnScreenResize(int width, int heigth) override;

	void HandleInput(SDL_Event* event);
    void SwitchViewPorts();
	void UnselectGameObject(GameObject* go);

	void Log(const char* input);
	void ClearConsole();

public:
	bool m_multipleViewports = false;

	bool m_isTestWindowOpen = false;

	bool m_show0Plane = true;
	bool m_renderNormals = false;
    std::string m_shaderResult;

private:
	UpdateStatus MenuBar();
	void Editor();
	void PlayButtons();
	void ViewPortUI(const ViewPort & port) const;
	void Console();
	void AttributeWindow();
	void Outliner();
	bool SaveLoadPopups();

	void SceneTreeGameObject(GameObject* node);

	void SelectByViewPort();
	void SelectGameObject(GameObject* node);
private:
	int m_screenW = 0;
	int m_screenH = 0;

	float2 m_viewPortMin;
	float2 m_viewPortMax;
	uint m_singleViewportIndex = 0;
	uint m_multipleViewportsIndex[2] = { 0,0 };
	uint m_fullScreenViewportIndex = 0;

	GameObject* m_selectedGameObject = nullptr;
	float m_selectedGoPos[3] = { 0,0,0 };
	float m_selectedGoScale[3] = { 1,1,1 };
	float m_selectedGoEuler[3] = { 0,0,0 };

	LineSegment m_selectionRay;
	float3 m_selectRayNormal;
	float3 m_selectedRayPos;
};

#endif