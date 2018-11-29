#include "ModuleSceneManager.h"

ModuleSceneManager::ModuleSceneManager()
	: Module()
{
}

ModuleSceneManager::~ModuleSceneManager()
{
}

bool ModuleSceneManager::Init()
{
	m_scenes.push_back(Scene("Global Scene"));
	m_scenes.push_back(Scene("Default Scene"));
	return true;
}

void ModuleSceneManager::Render(const ViewPort & port) const
{
	if (m_activeRenderScene != nullptr)
	{
		m_activeRenderScene->Render(port);
	}
}

void ModuleSceneManager::RenderGOs(const ViewPort & port) const
{
}
