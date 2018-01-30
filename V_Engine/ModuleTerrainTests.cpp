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

    uint w = 255;
    uint h = w;

    m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(w, h, 0, 0);

    for (uint y = 0; y < h; y++)
    {
        for (uint x = 0; x < w; x++)
        {
            float test = m_noiseMap[y][x];
        }
    }

    m_heightmapBuffer = VTerrain::GenImage::BlackAndWhite(m_noiseMap.Data(), m_noiseMap.Width(), m_noiseMap.Heigth(), true);

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleTerrain::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleTerrain::Update()
{
    ImGui::SetNextWindowPos(ImVec2(300.f, 50.f));
    if (ImGui::Begin("TerrainTests"))
    {
        float width = ImGui::GetWindowWidth() - 50;
        ImGui::Image((void*)m_heightmapBuffer, ImVec2(width, width));
        ImGui::End();
    }
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
    VTerrain::GenImage::FreeImage(m_heightmapBuffer);
	return true;
}
