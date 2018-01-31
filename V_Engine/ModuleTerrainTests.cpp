#include "Application.h"
#include "ModuleTerrainTests.h"

#include "ModuleInput.h"
#include "imGUI\imgui.h"

#include "OpenGL.h"

ModuleTerrain::ModuleTerrain(Application* app, bool start_enabled) :
    Module(app, start_enabled)
    , m_offset(0.f, 0.f)
    , m_size(64, 64)
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

    m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(m_size.x, m_size.y, m_offset.x, m_offset.y, { {1,1,1} });
    std::vector<float> tmp;
    tmp.resize(m_size.x * m_size.y * 3);
    for (int n = 0; n < m_noiseMap.Data().size(); n++)
    {
        tmp[n * 3 + 0] = m_noiseMap.Data()[n];
        tmp[n * 3 + 1] = m_noiseMap.Data()[n];
        tmp[n * 3 + 2] = m_noiseMap.Data()[n];
    }
    m_heightmapBuffer = VTerrain::GenImage::FromRGB(tmp, m_noiseMap.Width(), m_noiseMap.Heigth());

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
    bool regen = false;
    if (ImGui::Begin("TerrainTests"))
    {
        if (ImGui::Button("Add New Layer"))
        {
            m_layers.push_back(VTerrain::PerlinNoise::NoiseData(1.f, 1, 0.f));
        }
        if (ImGui::Button("Remove layer"))
        {
            m_layers.pop_back();
        }
        char tmp[256];
        for (int n = 0; n < m_layers.size(); n++)
        {
            ImGui::Text("Layer: ");
            sprintf(tmp, "Frequency##%i", n);
            if(ImGui::DragFloat(tmp, &m_layers[n].m_frequency, 0.1f, 0.1f, 64.f)) { regen = true; }
            sprintf(tmp, "Octaves##%i", n);
            if(ImGui::DragInt(tmp, &m_layers[n].m_octaves, 1, 1, 16)) { regen = true; }
            sprintf(tmp, "Strength##%i", n);
            if(ImGui::DragFloat(tmp, &m_layers[n].m_strength, 0.05f, 0.05f, 1.f)) { regen = true; }
            ImGui::Separator();
        }

        float width = ImGui::GetWindowWidth() - 50;
        ImGui::Image((void*)m_heightmapBuffer, ImVec2(width, width));
        ImGui::End();
    }

    if (App->input->GetKey(SDL_SCANCODE_KP_2) == KEY_REPEAT) { m_offset.y += 1; regen = true; }
    if (App->input->GetKey(SDL_SCANCODE_KP_8) == KEY_REPEAT) { m_offset.y -= 1; regen = true;}
    if (App->input->GetKey(SDL_SCANCODE_KP_6) == KEY_REPEAT) { m_offset.x += 1; regen = true;}
    if (App->input->GetKey(SDL_SCANCODE_KP_4) == KEY_REPEAT) { m_offset.x -= 1; regen = true;}

    if (regen)
    {
        VTerrain::GenImage::FreeImage(m_heightmapBuffer);

        m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(m_size.x, m_size.y, m_offset.x, m_offset.y, m_layers);
        std::vector<float> tmp;
        tmp.resize(m_size.x * m_size.y * 3);
        for (int n = 0; n < m_noiseMap.Data().size(); n++)
        {
            tmp[n * 3 + 0] = m_noiseMap.Data()[n];
            tmp[n * 3 + 1] = m_noiseMap.Data()[n];
            tmp[n * 3 + 2] = m_noiseMap.Data()[n];
        }
        m_heightmapBuffer = VTerrain::GenImage::FromRGB(tmp, m_noiseMap.Width(), m_noiseMap.Heigth());
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
