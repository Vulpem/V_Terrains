//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
#include "TerrainConfig.h"

#include "Chunk\Chunk.h"

namespace VTerrain
{
	float Config::maxHeight = 1000.f;
	unsigned int Config::chunkWidth = 64u;
	unsigned int Config::chunkHeight = 64u;
    unsigned int Config::maxChunks = 512u;
    unsigned int Config::nLODs = 8;
    float Config::quadSize = 6.f;

	float Config::globalLight[3] = { 0.2f, 0.2f, 0.2f };

	float Config::Noise::frequency = 0.8f;
	unsigned int Config::Noise::octaves = 8u;
	float Config::Noise::lacunarity = 2.f;
	float Config::Noise::persistency = 0.4f;

    unsigned int Config::TMP::debugTexBuf = 0;
    unsigned int Config::TMP::LOD = 0;

}