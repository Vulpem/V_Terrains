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
	, m_organisation(ORGANISATION)
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
	bool ret = true;
	// Call Init() in all modules
	std::vector<Module*>::iterator item = m_modules.begin();

	while(item != m_modules.end() && ret == true)
	{
		ret = (*item)->Init();
		item++;
	}

	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");
	item = m_modules.begin();

	//Variable used to determine if LOG's can be shown on console
	m_gameRunning = true;

	while(item != m_modules.end() && ret == true)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->Start();
		}
		item++;
	}
	m_maxFps = 0;

	m_msTimer.Start();
	m_fpsTimer.Start();
	m_totalTime.Start();

	//TMP
	TIMER_CREATE("__Timer");
	TIMER_CREATE_PERF("__PerfTimer");
	TIMER_CREATE_PERF("Timer Test");
	TIMER_CREATE_PERF("TimerPerf Test");

	return ret;
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
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();
	
	std::vector<Module*>::iterator item = m_modules.begin();
	
	while(item != m_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->PreUpdate();
		}
		item++;
	}
	TIMER_READ_MS("App PreUpdate");
	TIMER_START_PERF("App Update");
	item = m_modules.begin();

	while(item != m_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->Update();
		}
		item++;
	}
	TIMER_READ_MS("App Update");
	item = m_modules.begin();
	TIMER_START_PERF("App PostUpdate");
	while(item != m_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->PostUpdate();
		}
		item++;
	}
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

bool Application::CleanUp()
{
	m_gameRunning = false;

	bool ret = true;
	std::vector<Module*>::reverse_iterator item = m_modules.rbegin();

	while(item != m_modules.rend() && ret == true)
	{
		ret = (*item)->CleanUp();
		item++;
	}
	RELEASE(m_timers);

	return ret;
}

void Application::Render(const viewPort& port)
{
	std::vector<Module*>::iterator item = m_modules.begin();

	while (item != m_modules.end())
	{
		if ((*item)->IsEnabled())
		{
			(*item)->Render(port);
		}
		item++;
	}
}


bool Application::OpenBrowser(const char* link)
{
	ShellExecuteA(0, 0, "chrome.exe", link, 0, SW_SHOWMAXIMIZED);

	return true;
}

const char* Application::GetOrganization()
{
	return m_organisation.data();
}

const char* Application::GetTitle()
{
	return m_title.data();
}

void Application::OnScreenResize(int width, int heigth)
{
	for (std::vector<Module*>::iterator it = m_modules.begin(); it != m_modules.end(); it++)
	{
		(*it)->OnScreenResize(width, heigth);
	}
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
	std::for_each(m_modules.begin(), m_modules.end(),
		[](Module* m) {m->OnPlay(); });
}

void Application::Stop()
{
	Time.PlayMode = Play::Stop;
	std::for_each(m_modules.begin(), m_modules.end(),
		[](Module* m) {m->OnStop(); });
}

void Application::AddModule(Module* mod)
{
	m_modules.push_back(mod);
}