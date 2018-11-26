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
#include "ModuleSceneManager.h"
#include "ModuleTerrainTests.h"
#include <algorithm>

Application::Application()
	: m_title(TITLE)
{
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
	// The order of calls is important
	// Modules will Init() OnEnable() and Update in this order
	// They will OnDisable() in reverse order

	CreateModule<ModuleWindow>(m_window);
	CreateModule<ModuleInput>(m_input);
	CreateModule<ModuleCamera3D>(m_camera);
#if USE_EDITOR
	CreateModule<ModuleEditor>(m_editor);
#endif
	CreateModule<ModuleFileSystem>(m_fileSystem);
	CreateModule<ModuleAudio>(m_audio);
	CreateModule<ModulePhysics3D>(m_physics);
	CreateModule<ModuleImporter>(m_importer);
	CreateModule<ModuleResourceManager>(m_resourceManager);
	CreateModule<ModuleSceneManager>(m_scenes);
	CreateModule<ModuleGoManager>(m_goManager);
	CreateModule<ModuleTerrain>(m_terrain);
	// Renderer last!
	CreateModule<ModuleRenderer3D>(m_renderer3D);

	// After all Init calls we call OnEnable() in all modules
	LOG("Application Start --------------");
	//Variable used to determine if LOG's can be shown on console
	for (Module* m : m_modules)
	{
		m->Enable();
	}
	m_maxFps = 0;
	TIMER_START("Total Runtime");
	TIMER_START("Timer new second");

	return true;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	TIMER_START_PERF("App PreUpdate");
	m_frameCount++;

	//Time managing
	Time.dt = TIMER_READ_MS("frame time") / 1000.0f;
	if (Time.PlayMode != PlayMode::Stop && Time.Pause == false)
	{
		Time.gdt = Time.dt / Time.gdtModifier;
		Time.GameRuntime += Time.dt;
	}
	else
	{
		Time.gdt = 0.0f;
	}
	Time.AppRuntime = TIMER_READ_MS("Total Runtime")/1000.0f;
	//

	TIMER_START_PERF("frame time");

	for (int n = 0; n < EDITOR_FRAME_SAMPLES - 1; n++)
	{
		m_msFrame[n] = m_msFrame[n + 1];
	}
	m_msFrame[EDITOR_FRAME_SAMPLES - 1] = Time.dt;

	if (TIMER_READ_MS("Timer new second") > 1000.0f)
	{
		for (int n = 0; n < EDITOR_FRAME_SAMPLES - 1; n++)
		{
			m_framerate[n] = m_framerate[n + 1];
		}
		m_framerate[EDITOR_FRAME_SAMPLES-1] = m_frameCount;
		m_frameCount = 0;
		TIMER_START("Timer new second");
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
		while (TIMER_READ_MS("frame time") < m_frameTime)
		{
		}
	}
	return ret;
}

void Application::OnDisable()
{
	for (Module* m : m_modules)
	{
		m->OnDisable();
	};
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
		Time.PlayMode = PlayMode::Play;
	}
	else
	{
		Time.PlayMode = PlayMode::DebugPlay;
	}
	for (Module* m : m_modules) {m->OnPlay(); };
}

void Application::Stop()
{
	Time.PlayMode = PlayMode::Stop;
	for (Module* m : m_modules) {m->OnStop(); };
}

void Application::Log(const char * str)
{
		if (m_editor != nullptr && m_editor->IsEnabled())
		{
			m_editor->Log(str);
		}
}