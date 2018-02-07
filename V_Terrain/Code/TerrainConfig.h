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
        static void SetMaxChunks(unsigned int maxChunks);
        static unsigned int GetMaxChunks() { return maxChunks; }
    private:
        static unsigned int maxChunks;
    public:

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