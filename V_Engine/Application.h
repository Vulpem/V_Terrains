#ifndef __APP__
#define __APP__

#include "Globals.h"
#include "TimerManager.h"

#include <vector>
#include <list>

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleEditor;
class ModuleGoManager;
class ModulePhysics3D;
class ModuleFileSystem;
class ModuleImporter;
class ModuleResourceManager;
class ModuleTerrain;
class ModuleSceneManager;

struct ViewPort;

#include "MathGeoLib\include\MathGeoLibFwd.h"

class Application
{
public:
	Application();
	~Application();

	bool Init();
	UpdateStatus Update();
	void OnDisable();

	void Render(const ViewPort& port) const;

	bool OpenBrowser(const char* link) const;

	const char* GetTitle() const;
	void OnScreenResize(int width, int heigth);
	void Play(bool debug = false);
	void Stop();
	void Log(const char* str);

public:
	//All accessible modules
	ModuleWindow * m_window = nullptr;
	ModuleInput* m_input = nullptr;
	ModuleAudio* m_audio = nullptr;
	ModuleFileSystem* m_fileSystem = nullptr;

	ModuleRenderer3D* m_renderer3D = nullptr;
	ModuleCamera3D* m_camera = nullptr;

	ModuleEditor* m_editor = nullptr;
	ModuleResourceManager* m_resourceManager = nullptr;
	ModuleImporter* m_importer = nullptr;
	ModuleSceneManager* m_scenes = nullptr;
	ModuleGoManager* m_goManager = nullptr;
	ModulePhysics3D* m_physics = nullptr;
	ModuleTerrain* m_terrain = nullptr;

	//Other variables
	float m_msFrame[EDITOR_FRAME_SAMPLES];
	float m_framerate[EDITOR_FRAME_SAMPLES];
	int m_maxFps = 30;

	TimerManager m_timers;

private:
	template <typename ModuleType>
	void CreateModule(ModuleType*& modulePointer);
	void PrepareUpdate();
	void FinishUpdate();

private:
	std::vector<Module*> m_modules;
	int m_frameCount = 0;
	float m_frameTime = -1.0f;
	int m_previousMaxFps = m_maxFps;

	std::string m_title;
};

extern Application* App;

template<typename ModuleType>
void Application::CreateModule(ModuleType*& modulePointer)
{
	modulePointer = new ModuleType();
	m_modules.push_back(modulePointer);
	modulePointer->Init();
}

#endif