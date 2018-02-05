#include "Application.h"
#include "ModuleTerrainTests.h"

#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ViewPort.h"

#include "ModuleInput.h"
#include "imGUI\imgui.h"

#include "OpenGL.h"

ModuleTerrain::ModuleTerrain(Application* app, bool start_enabled) :
    Module(app, start_enabled)
    , m_offset(0.f, 0.f)
    , m_size(75, 75)
    , m_frequency(10.f)
    , m_octaves(5)
    , m_lacunarity(2.f)
    , m_persistance (0.25f)
    , m_simplifyRender(false)
    , m_simplifyRenderStep(0.2f)
    , m_maxHeight(15.f)
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
    VTerrain::Init();

    std::vector<float> tmp = { 0.f,0.f,0.f };
    m_heightmapBuffer = VTerrain::GenImage::FromRGB(tmp, 1, 1);
    GenMap();

    VTerrain::Shaders::CompileShader(nullptr, nullptr, m_shaderProgram);

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
        if (ImGui::DragFloat("MaxHeight", &m_maxHeight, 0.1f, 0.1f, 64.f)) { regen = true; }

        if (ImGui::DragFloat2("Size", &m_size[0], 1.f, 5.f, 1024.f)) { regen = true; }

        if (ImGui::DragFloat2("Offset", &m_offset[0], 1.f)) { regen = true; }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Frequency", &m_frequency, 0.1f, 64.f)) { regen = true; }
        if (ImGui::DragFloat("Frequency", &m_frequency, 0.1f, 0.1f, 64.f)) { regen = true; }
        ImGui::Separator();

        if (ImGui::SliderInt("##Octaves", &m_octaves, 1, 16)) { regen = true; }
        if (ImGui::DragInt("Octaves", &m_octaves, 1, 1, 16)) { regen = true; }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Lacunarity", &m_lacunarity, 0.1f, 20.f)) { regen = true; }
        if (ImGui::DragFloat("Lacunarity", &m_lacunarity, 0.1f, 0.1f, 20.f)) { regen = true; }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Persistance", &m_persistance, 0.01f, 1.f)) { regen = true; }
        if (ImGui::DragFloat("Persistance", &m_persistance, 0.1f, 0.01f, 1.f)) { regen = true; }
        ImGui::Separator();

        if (ImGui::Checkbox("Simplify render", &m_simplifyRender)) { regen = true; }
        if (ImGui::SliderFloat("Simplify render step", &m_simplifyRenderStep, 0.01f, 1.f)) { regen = true; }
        ImGui::NewLine();

        float width = ImGui::GetWindowWidth() - 50;
        ImGui::Image((void*)m_heightmapBuffer, ImVec2(width, width));
        ImGui::End();
    }

    if (App->input->GetKey(SDL_SCANCODE_KP_5) == KEY_REPEAT) { m_offset.y += 1; regen = true; }
    if (App->input->GetKey(SDL_SCANCODE_KP_8) == KEY_REPEAT) { m_offset.y -= 1; regen = true;}
    if (App->input->GetKey(SDL_SCANCODE_KP_6) == KEY_REPEAT) { m_offset.x += 1; regen = true;}
    if (App->input->GetKey(SDL_SCANCODE_KP_4) == KEY_REPEAT) { m_offset.x -= 1; regen = true;}

    if (regen)
    {
        GenMap();
    }
    
    m_mesh.Render(App->camera->GetDefaultCam()->GetViewMatrix().ptr(), App->camera->GetDefaultCam()->GetProjectionMatrix().ptr());

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

void ModuleTerrain::GenMap()
{
    VTerrain::Config::chunkHeight = m_size.x;
    VTerrain::Config::chunkWidth = m_size.y;
    VTerrain::Config::maxHeight = m_maxHeight;

    VTerrain::Config::Noise::frequency = m_frequency;
    VTerrain::Config::Noise::octaves = m_octaves;
    VTerrain::Config::Noise::lacunarity = m_lacunarity;
    VTerrain::Config::Noise::persistency = m_persistance;


    m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(m_offset.x, m_offset.y);

    VTerrain::MeshGenerator::MeshData meshData;
    meshData.Generate(m_noiseMap);

    m_mesh.Generate(meshData);
    
    VTerrain::GenImage::FreeImage(m_heightmapBuffer);

    std::vector<float> tmp;
    tmp.resize(m_size.x * m_size.y * 3);
    for (int n = 0; n < m_noiseMap.Data().size(); n++)
    {
        float val = m_noiseMap.Data()[n];
        if (m_simplifyRender)
        {
            for (float d = 1 - m_simplifyRenderStep; d >= -1.f; d -= m_simplifyRenderStep)
            {
                if (val >= d && val < d + m_simplifyRenderStep)
                {
                    val = Max(d + m_simplifyRenderStep, 0.f); break;
                }
            }
        }
        tmp[n * 3 + 0] = val;
        tmp[n * 3 + 1] = val;
        tmp[n * 3 + 2] = val;
    }
    m_heightmapBuffer = VTerrain::GenImage::FromRGB(tmp, m_noiseMap.Width(), m_noiseMap.Height());
}
