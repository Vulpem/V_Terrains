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

    void GenMap();

    VTerrain::PerlinNoise::NoiseMap m_noiseMap;
    VTerrain::MeshGenerator::Mesh m_mesh;

    //NoiseGeneration
    uint32_t seed;

    uint m_heightmapBuffer = 0;

    float m_frequency;
    int m_octaves;
    float m_lacunarity;
    float m_persistance;

    float2 m_size;
    float2 m_offset;

    bool m_simplifyRender;
    float m_simplifyRenderStep;

    uint m_shaderProgram;
};

#endif