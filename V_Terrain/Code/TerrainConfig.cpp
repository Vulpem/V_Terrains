#include "TerrainConfig.h"

#include "Chunk\Chunk.h"

namespace VTerrain
{
	float Config::maxHeight = 100.f;
	unsigned int Config::chunkWidth = 128u;
	unsigned int Config::chunkHeight = 128u;
    unsigned int Config::maxChunks = 64;

	float Config::globalLight[3] = { 0.2f, 0.2f, 0.2f };

	float Config::Noise::frequency = 0.8f;
	unsigned int Config::Noise::octaves = 8u;
	float Config::Noise::lacunarity = 2.f;
	float Config::Noise::persistency = 0.4f;

    unsigned int Config::TMP::debugTexBuf = 0;

    void Config::SetMaxChunks(unsigned int max)
    {
        maxChunks = max;
    }
}