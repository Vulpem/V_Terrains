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
//  You should have received a copy of the GNU General Public License along with V Terrains.  If not, see <http://www.gnu.org/licenses/>.
#ifndef __V__TERRAIN__CONFIG__
#define __V__TERRAIN__CONFIG__

#include <functional>

namespace VTerrain
{
    struct Config
    {
        float maxHeight = 1000.f;
        unsigned int chunkHeightmapResolution = 32u;
        unsigned int maxChunks = 512u;
        float chunkSize = 1024.f;
        unsigned int chunkMinDensity = 10;
		float fogDistance = 10000.f;
		float waterHeight = 0.004f;
        float ambientLight = 0.4f;

		bool singleSidedFaces = true;

		float waterColor[3] = { 0.137f, 0.537f, 0.855f };
		float fogColor[3] = { 0.78f, 0.81f, 0.84f };
		float globalLight[3] = { 0.2f, -0.2f, 0.2f };
		struct Noise
        {
			unsigned int ridgedDepth = 1;
			float frequency = 0.404f;
			unsigned int octaves = 5u;
			float lacunarity = 2.82f;
			float persistency = 0.428f;
        } noise;

		struct Debug
        {
			bool wiredRender = false;
			bool renderLight = true;
            bool renderHeightmap = false;
            bool renderChunkBorders = false;
        } debug;
		std::function<void(const char*)> throwErrorFunc;
    };
    extern Config config;
}
#endif