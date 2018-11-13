//  RPG Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "RPG Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "RPG Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with RPG Terrains.  If not, see <http://www.gnu.org/licenses/>.
#ifndef __V__TERRAIN__CONFIG__
#define __V__TERRAIN__CONFIG__

#include <functional>

namespace RPGT
{
    struct Config
    {
        unsigned int maxChunks = 512u;
        float chunkSize = 1024.f;
        unsigned int chunkHeightmapResolution = 32u;
        unsigned int chunkMinDensity = 10;

        float maxHeight = 1000.f;
		float waterHeight = 0.004f;

        float fogDistance = 10000.f;
		float fogColor[3] = { 0.78f, 0.81f, 0.84f };

        float ambientLight = 0.4f;
		float globalLight[3] = { 0.2f, -0.2f, 0.2f };
        bool singleSidedFaces = true;

		float tesselationTriangleSize = 4.25f;
		int screenWidth = 1920;
		int screenHeight = 1080;

		struct Noise
        {
			unsigned int ridgedDepth = 2;
			float frequency = 0.404f;
			unsigned int octaves = 5u;
			float lacunarity = 2.0f;
			float persistency = 0.428f;
        } noise;

		struct Debug
        {
			bool wiredRender = false;
			bool renderLight = true;
            bool renderHeightmap = false;
            bool renderChunkBorders = false;
        } debug;

        std::function<float(float)> m_heightCurve;
		std::function<void(const char*)> throwErrorFunc;
		std::function<void(int x, int y)> chunkLoaded;
		std::function<void(int x, int y)> chunkUnloaded;

		Config()
			: m_heightCurve([](float a) {return a; })
			, throwErrorFunc([](const char*) {})
			, chunkLoaded([](int, int) {})
			, chunkUnloaded([](int, int) {})
		{}
    };
    extern Config config;
}
#endif