#include "TerrainConfig.h"

namespace VTerrain
{
	float Config::maxHeight = 15.f;
	unsigned int Config::chunkWidth = 50u;
	unsigned int Config::chunkHeight = 50u;

	float Config::Noise::frequency = 10.f;
	unsigned int Config::Noise::octaves = 5u;
	float Config::Noise::lacunarity = 3.f;
	float Config::Noise::persistency = 0.4f;
}