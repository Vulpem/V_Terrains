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
