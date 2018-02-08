#include "TerrainConfig.h"

#include "Chunk\Chunk.h"

namespace VTerrain
{
	float Config::maxHeight = 15.f;
	unsigned int Config::chunkWidth = 50u;
	unsigned int Config::chunkHeight = 50u;
    unsigned int Config::maxChunks = 25;

	float Config::globalLight[3] = { 0.2f, 0.2f, 0.2f };

	float Config::Noise::frequency = 10.f;
	unsigned int Config::Noise::octaves = 5u;
	float Config::Noise::lacunarity = 3.f;
	float Config::Noise::persistency = 0.4f;

    unsigned int Config::TMP::debugTexBuf = 0;

    void Config::SetMaxChunks(unsigned int max)
    {
        maxChunks = max;
    }
}