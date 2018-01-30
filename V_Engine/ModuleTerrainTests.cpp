#include "Application.h"
#include "ModuleTerrainTests.h"

#include "ModuleInput.h"
#include "imGUI\imgui.h"

#include "OpenGL.h"

ModuleTerrain::ModuleTerrain(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	moduleName = "ModuleTerrainTests";
}

// Destructor
ModuleTerrain::~ModuleTerrain()
{}

// Called before render is available
bool ModuleTerrain::Init()
{
    bool ret = true;
	return ret;
}

bool ModuleTerrain::Start()
{
	bool ret = true;

    uint w = 10;
    uint h = w;

    m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(w, h, 0, 0);

    for (uint y = 0; y < h; y++)
    {
        for (uint x = 0; x < w; x++)
        {
            float test = m_noiseMap[y][x];
        }
    }


	return ret;
}

// PreUpdate: clear buffer
update_status ModuleTerrain::PreUpdate()
{
	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleTerrain::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleTerrain::CleanUp()
{

	return true;
}
