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

namespace VTerrain
{
    struct Config
    {
	public:
        float maxHeight = 1000.f;
        unsigned int chunkWidth = 64u;
        unsigned int chunkHeight = 64u;
        unsigned int maxChunks = 256u;
        unsigned int nLODs = 6;
        float quadSize = 6.f;
		float fogDistance = 10000.f;
		float waterHeight = 330.f;
        float ambientLight = 0.4f;

		float waterColor[3] = { 0.137f, 0.537f, 0.855f };
		float fogColor[3] = { 0.78f, 0.81f, 0.84f };
		float globalLight[3] = { 0.2f, -0.2f, 0.2f };
		struct Noise
        {
			float frequency = 0.8f;
			unsigned int octaves = 8u;
			float lacunarity = 2.f;
			float persistency = 0.4f;
        } noise;

		struct TMP
        {
            unsigned int LOD = 0;;
        } tmp;
   };

	extern Config config;
}
#endif