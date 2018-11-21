#ifndef __APP__
#define __APP__

#include "Globals.h"
#include "Timers.h"

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
	ModuleWindow * m_window;
	ModuleInput* m_input;
	ModuleAudio* m_audio;
	ModuleFileSystem* m_fileSystem;

	ModuleRenderer3D* m_renderer3D;
	ModuleCamera3D* m_camera;

	ModuleEditor* m_editor;
	ModuleResourceManager* m_resourceManager;
	ModuleImporter* m_importer;
	ModuleGoManager* m_goManager;
	ModulePhysics3D* m_physics;
	ModuleTerrain* m_terrain;

	//Other variables
	float m_msFrame[EDITOR_FRAME_SAMPLES];
	float m_framerate[EDITOR_FRAME_SAMPLES];
	int m_maxFps = 30;

	TimerManager m_timers;
	Timer m_totalTime;

private:
	template <typename ModuleType>
	void CreateModule(ModuleType*& modulePointer);
	void PrepareUpdate();
	void FinishUpdate();

private:
	PerfTimer	m_msTimer;
	std::vector<Module*> m_modules;
	int m_frameCount = 0;
	Timer	m_fpsTimer;
	float m_frameTime = -1.0f;
	int m_previousMaxFps = m_maxFps;
	bool m_gameRunning = false;

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