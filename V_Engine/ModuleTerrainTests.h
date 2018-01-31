#ifndef __MODULE_TERRAIN_TESTS__
#define __MODULE_TERRAIN_TESTS__

#include "Module.h"
#include "Globals.h"

#include "../V_Terrain/Code/Include.h"

#include "Math.h"

class ModuleTerrain : public Module
{
public:
    ModuleTerrain(Application* app, bool start_enabled = true);
	~ModuleTerrain();

	bool Init();
	bool Start();
	update_status PreUpdate() override;
    update_status Update() override;
	update_status PostUpdate() override;
	bool CleanUp();

    VTerrain::PerlinNoise::NoiseMap m_noiseMap;

    int m_heightmapBuffer = 0;

    float m_frequency;
    int m_octaves;
    std::vector<VTerrain::PerlinNoise::NoiseData> m_layers;

    float2 m_size;
    float2 m_offset;
};

#endif