#include "Application.h"

#include <time.h>


#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleEditor.h"
#include "ModuleGoManager.h"
#include "ModuleFileSystem.h"
#include "ModuleImporter.h"
#include "ModuleResourceManager.h"
#include "ModuleTerrainTests.h"
#include <algorithm>

#include "Timers.h"

Application::Application()
	: m_gameRunning(false)
	, m_title(TITLE)
{
	m_window = new ModuleWindow(this);
	m_input = new ModuleInput(this);
	m_audio = new ModuleAudio(this, true);
	m_fileSystem = new ModuleFileSystem(this);

	m_renderer3D = new ModuleRenderer3D(this);
	m_camera = new ModuleCamera3D(this);
	m_physics = new ModulePhysics3D(this);
#if USE_EDITOR
	m_editor = new ModuleEditor(this);
#endif
	m_resourceManager = new ModuleResourceManager(this);
	m_goManager = new ModuleGoManager(this);
	m_importer = new ModuleImporter(this);
	m_terrain = new ModuleTerrain(this);

	m_timers = new TimerManager();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(m_window);
	AddModule(m_input);
	AddModule(m_camera);
#if USE_EDITOR
		AddModule(m_editor);
#endif
	AddModule(m_fileSystem);
	AddModule(m_audio);
	AddModule(m_physics);
	AddModule(m_importer);
	AddModule(m_resourceManager);
	AddModule(m_goManager);
    AddModule(m_terrain);

	// Renderer last!
	AddModule(m_renderer3D);

	for (int n = 0; n < EDITOR_FRAME_SAMPLES; n++)
	{
		m_msFrame[n] = 0;
		m_framerate[n] = 0;
	}
	m_frameTime = -1.0f;

	srand(time(NULL));
}

Application::~Application()
{
	std::vector<Module*>::reverse_iterator item = m_modules.rbegin();

	while(item != m_modules.rend())
	{
		delete *item;
		item++;
	}
}

bool Application::Init()
{
	for(Module* m : m_modules)
	{
		if (m->Init() == false)
		{
			return false;
		}
	};

	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");
	//Variable used to determine if LOG's can be shown on console
	m_gameRunning = true;
	for (Module* m : m_modules)
	{
		if (m->IsEnabled())
		{
			m->Start();
		}
	};
	m_maxFps = 0;
	m_msTimer.Start();
	m_fpsTimer.Start();
	m_totalTime.Start();

	//TMP
	TIMER_CREATE("__Timer");
	TIMER_CREATE_PERF("__PerfTimer");
	TIMER_CREATE_PERF("Timer Test");
	TIMER_CREATE_PERF("TimerPerf Test");

	return true;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	TIMER_START_PERF("App PreUpdate");
	m_frameCount++;

	//Time managing
	Time.dt = m_msTimer.ReadMs() / 1000.0f;
	if (Time.PlayMode != Play::Stop && Time.Pause == false)
	{
		Time.gdt = Time.dt / Time.gdtModifier;
		Time.GameRuntime += Time.dt;
	}
	else
	{
		Time.gdt = 0.0f;
	}
	Time.AppRuntime = m_totalTime.Read() / 1000.0f;
	//

	m_msTimer.Start();

	for (int n = 0; n < EDITOR_FRAME_SAMPLES - 1; n++)
	{
		m_msFrame[n] = m_msFrame[n + 1];
	}
	m_msFrame[EDITOR_FRAME_SAMPLES - 1] = Time.dt;

	float tmp = m_fpsTimer.Read();
	if (m_fpsTimer.Read() > 1000.0f)
	{
		for (int n = 0; n < EDITOR_FRAME_SAMPLES - 1; n++)
		{
			m_framerate[n] = m_framerate[n + 1];
		}
		m_framerate[EDITOR_FRAME_SAMPLES-1] = m_frameCount;
		m_frameCount = 0;
		m_fpsTimer.Start();
	}
	
	if (m_maxFps != m_previousMaxFps)
	{
		if (m_maxFps < 5)
		{
			m_frameTime = -1.0f;
		}
		else
		{
			m_frameTime = 1000.0f / m_maxFps;
		}
		m_previousMaxFps = m_maxFps;
	}
}

// ---------------------------------------------
void Application::FinishUpdate()
{
}

// Call PreUpdate, Update and PostUpdate on all modules
UpdateStatus Application::Update()
{
	UpdateStatus ret = UpdateStatus::Continue;
	PrepareUpdate();
	
	for (Module* m : m_modules)
	{
		if (m->IsEnabled() && ret == UpdateStatus::Continue)
		{
			ret = m->PreUpdate();
		}
	};
	TIMER_READ_MS("App PreUpdate");
	TIMER_START_PERF("App Update");
	for (Module* m : m_modules)
	{
		if (m->IsEnabled() && ret == UpdateStatus::Continue)
		{
			ret = m->Update();
		}
	};
	TIMER_READ_MS("App Update");
	TIMER_START_PERF("App PostUpdate");
	for (Module* m : m_modules)
	{
		if (m->IsEnabled() && ret == UpdateStatus::Continue)
		{
			ret = m->PostUpdate();
		}
	};
	TIMER_READ_MS("App PostUpdate");
	FinishUpdate();
	if (m_frameTime > 0.0001f)
	{
		while (m_msTimer.ReadMs() < m_frameTime)
		{
		}
	}
	return ret;
}

void Application::CleanUp()
{
	m_gameRunning = false;
	std::vector<Module*>::reverse_iterator item = m_modules.rbegin();
	for (Module* m : m_modules)
	{
		m->CleanUp();
	};
	RELEASE(m_timers);
}

void Application::Render(const ViewPort& port) const
{
	for (Module* m : m_modules)
	{
		if (m->IsEnabled())
		{
			m->Render(port);
		}
	};
}


bool Application::OpenBrowser(const char* link) const
{
	ShellExecuteA(0, 0, "chrome.exe", link, 0, SW_SHOWMAXIMIZED);

	return true;
}

const char* Application::GetTitle() const
{
	return m_title.data();
}

void Application::OnScreenResize(int width, int heigth)
{
	for (Module* m : m_modules) {m->OnScreenResize(width, heigth); };
}

void Application::Play(bool debug)
{
	if (!debug)
	{
		Time.PlayMode = Play::Play;
	}
	else
	{
		Time.PlayMode = Play::DebugPlay;
	}
	for (Module* m : m_modules) {m->OnPlay(); };
}

void Application::Stop()
{
	Time.PlayMode = Play::Stop;
	for (Module* m : m_modules) {m->OnStop(); };
}

void Application::AddModule(Module* mod)
{
	m_modules.push_back(mod);
}