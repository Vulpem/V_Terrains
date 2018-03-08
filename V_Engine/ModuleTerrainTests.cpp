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
    , m_size(128, 128)
    , m_frequency(0.8f)
    , m_octaves(8)
    , m_lacunarity(2.0f)
    , m_persistance (0.4f)
    , m_maxHeight(1000.f)
	, m_fogDistance(10000.f)
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
   
    GenMap();

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleTerrain::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleTerrain::Update()
{
    float3 pos = App->camera->GetDefaultCam()->GetPosition();
	VTerrain::Update(pos.x, pos.z);

    ImGui::SetNextWindowPos(ImVec2(0.f, 20.f));

    if (ImGui::Begin("TerrainTests"))
    {
        const int W = static_cast<int>(VTerrain::config.chunkWidth);
        const int H = static_cast<int>(VTerrain::config.chunkHeight);
        int p[2] = {
            floor((pos.x - floor(W / 2.f) + (W % 2 != 0)) / W + 1),
            floor((pos.z - floor(H / 2.f) + (H % 2 != 0)) / H + 1)
        };
        ImGui::DragInt2("CurrentChunk", p);

        int tmpLOD = VTerrain::config.tmp.LOD;
        if (ImGui::SliderInt("LOD", &tmpLOD, 0,VTerrain::config.nLODs - 1))
        {
            VTerrain::config.tmp.LOD = tmpLOD;
        }

        bool lightDirChanged = false;
        if (ImGui::SliderFloat("LightDir", &m_globalLightDir, -360, 360)) { lightDirChanged = true; }
        if (ImGui::SliderFloat("LightHeight", &m_globalLightHeight, -90, 90)) { lightDirChanged = true; }
		
        if(lightDirChanged)
        {
			float3 tmp(1.f, 0.f, 0.f); //cos(m_globalLightDir * DEGTORAD),0 ,sin(m_globalLightDir* DEGTORAD));
			Quat rot = Quat::FromEulerYZX(m_globalLightDir * DEGTORAD, -m_globalLightHeight*DEGTORAD, 0.f);
			tmp = rot * tmp;
			VTerrain::config.globalLight[0] = tmp.x;
			VTerrain::config.globalLight[1] = tmp.y;
			VTerrain::config.globalLight[2] = tmp.z;
		}

        ImGui::DragFloat3("GlobalLightDirection", VTerrain::config.globalLight);

        if (ImGui::SliderFloat("MaxHeight", &m_maxHeight, 0.0f, 8000.f, "%.3f", 3.f)) { VTerrain::config.maxHeight = m_maxHeight; }
		ImGui::SliderFloat("FogDistance", &VTerrain::config.fogDistance, 0.0f, 100000.f, "%.3f", 4.f);
		ImGui::SliderFloat("WaterHeight", &VTerrain::config.waterHeight, 0.0f, m_maxHeight, "%.3f", 4.f);


        if (ImGui::DragFloat2("Size", &m_size[0], 1.f, 5.f, 1024.f)) { WantRegen(); }

        if (ImGui::DragFloat2("Offset", &m_offset[0], 1.f)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Frequency", &m_frequency, 0.1f, 20.f)) { WantRegen(); }
        if (ImGui::DragFloat("Frequency", &m_frequency, 0.1f, 0.1f, 64.f)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderInt("##Octaves", &m_octaves, 1, 16)) { WantRegen(); }
        if (ImGui::DragInt("Octaves", &m_octaves, 1, 1, 16)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Lacunarity", &m_lacunarity, 0.1f, 20.f)) { WantRegen(); }
        if (ImGui::DragFloat("Lacunarity", &m_lacunarity, 0.1f, 0.1f, 20.f)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Persistance", &m_persistance, 0.01f, 1.f)) { WantRegen(); }
        if (ImGui::DragFloat("Persistance", &m_persistance, 0.1f, 0.01f, 1.f)) { WantRegen(); }
		ImGui::End();
    }

	if (m_wantRegen && m_regenTimer.Read() > 2000.0f)
	{
		m_regenTimer.Stop();
		m_wantRegen = false;
		GenMap();
		VTerrain::CleanChunks();
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
	return true;
}

void ModuleTerrain::Render(const viewPort & port)
{
	VTerrain::Render(port.camera->GetViewMatrix().ptr(), port.camera->GetProjectionMatrix().ptr());
}

void ModuleTerrain::GenMap()
{
    VTerrain::config.chunkHeight = m_size.x;
    VTerrain::config.chunkWidth = m_size.y;
   

    VTerrain::config.noise.frequency = m_frequency;
    VTerrain::config.noise.octaves = m_octaves;
    VTerrain::config.noise.lacunarity = m_lacunarity;
    VTerrain::config.noise.persistency = m_persistance;
}

void ModuleTerrain::WantRegen()
{
	m_wantRegen = true;
	m_regenTimer.Start();
}
