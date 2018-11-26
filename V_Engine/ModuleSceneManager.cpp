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
	return false;
}

UpdateStatus ModuleSceneManager::PreUpdate()
{
	return UpdateStatus::Continue;
}

UpdateStatus ModuleSceneManager::Update()
{
	return UpdateStatus::Continue;
}

UpdateStatus ModuleSceneManager::PostUpdate()
{
	return UpdateStatus::Continue;
}

void ModuleSceneManager::OnEnable()
{
}

void ModuleSceneManager::OnDisable()
{
}

void ModuleSceneManager::Render(const ViewPort & port) const
{
}
