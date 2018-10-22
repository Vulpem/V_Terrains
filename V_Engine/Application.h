#ifndef __APP__
#define __APP__

#include "Globals.h"
#include "Timer.h"
#include "PerfTimer.h"

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
class TimerManager;
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
	bool CleanUp();

	void Render(const ViewPort& port);

	bool OpenBrowser(const char* link);

	const char* GetTitle();
	void OnScreenResize(int width, int heigth);
	void Play(bool debug = false);
	void Stop();

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

	TimerManager* m_timers;
	Timer m_totalTime;

private:

	void AddModule(Module* mod);
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

#endif