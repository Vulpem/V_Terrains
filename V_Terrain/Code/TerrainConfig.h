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
#ifndef __V__TERRAIN__CONFIG__
#define __V__TERRAIN__CONFIG__

namespace VTerrain
{
    class Config
    {
	public:
        static float maxHeight;
        static unsigned int chunkWidth;
        static unsigned int chunkHeight;
        static unsigned int maxChunks;
        static unsigned int nLODs;

		static float globalLight[3];
        class Noise
        {
		public:
			static float frequency;
			static unsigned int octaves;
			static float lacunarity;
			static float persistency;
        };

        class TMP
        {
        public:
            static unsigned int debugTexBuf;
        };
   };
}
#endif