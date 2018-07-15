//----------------------------------------------------------------------------------------
//
//	siv::PerlinNoise
//	Perlin noise library for modern C++
//
//	Copyright (C) 2013-2016 Ryo Suzuki <reputeless@gmail.com>
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files(the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions :
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//	
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
//----------------------------------------------------------------------------------------

# pragma once
# include <cstdint>
# include <numeric>
# include <algorithm>
# include <random>
#include <functional>

namespace siv
{
    class SivPerlinNoise
    {
    private:

        std::int32_t p[512];

        static float Fade(float t) noexcept
        {
            return t * t * t * (t * (t * 6 - 15) + 10);
        }

        static float Lerp(float t, float a, float b) noexcept
        {
            return a + t * (b - a);
        }

        static float Grad(std::int32_t hash, float x, float y, float z) noexcept
        {
            const std::int32_t h = hash & 15;
            const float u = h < 8 ? x : y;
            const float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        }

    public:

        explicit SivPerlinNoise(std::uint32_t seed = std::default_random_engine::default_seed)
        {
            reseed(seed);
        }

        void reseed(std::int32_t seed)
        {
            for (int32_t i = 0; i < 256; ++i)
            {
                p[i] = i;
            }

            std::shuffle(std::begin(p), std::begin(p) + 256, std::default_random_engine(seed));

            for (int32_t i = 0; i < 256; ++i)
            {
                p[256 + i] = p[i];
            }
        }

        float noise(float x) const
        {
            return noise(x, 0.0f, 0.0f);
        }

        float noise(float x, float y) const
        {
            return noise(x, y, 0.0f);
        }

        float noise(float x, float y, float z) const
        {
            const std::int32_t X = static_cast<std::int32_t>(std::floor(x)) & 255;
            const std::int32_t Y = static_cast<std::int32_t>(std::floor(y)) & 255;
            const std::int32_t Z = static_cast<std::int32_t>(std::floor(z)) & 255;

            x -= std::floor(x);
            y -= std::floor(y);
            z -= std::floor(z);

            const float u = Fade(x);
            const float v = Fade(y);
            const float w = Fade(z);

            const std::int32_t A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
            const std::int32_t B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

            return Lerp(w, Lerp(v, Lerp(u, Grad(p[AA], x, y, z),
                Grad(p[BA], x - 1, y, z)),
                Lerp(u, Grad(p[AB], x, y - 1, z),
                    Grad(p[BB], x - 1, y - 1, z))),
                Lerp(v, Lerp(u, Grad(p[AA + 1], x, y, z - 1),
                    Grad(p[BA + 1], x - 1, y, z - 1)),
                    Lerp(u, Grad(p[AB + 1], x, y - 1, z - 1),
                        Grad(p[BB + 1], x - 1, y - 1, z - 1))));
        }

        float octaveNoise(float x, std::int32_t octaves, float lacunarity = 2.f, float persistance = 0.25f) const
        {
            float result = 0.0f;
            float amp = 1.0f;

            for (std::int32_t i = 0; i < octaves; ++i)
            {
                result += noise(x) * amp;
                x *= lacunarity;
                amp *= persistance;
            }

            return result;
        }

        float octaveNoise(float x, float y, std::int32_t octaves, float lacunarity = 2.f, float persistance = 0.25f) const
        {
            float result = 0.0f;
            float amp = 1.0f;

            for (std::int32_t i = 0; i < octaves; ++i)
            {
                result += noise(x, y) * amp;
                x *= lacunarity;
                y *= lacunarity;
                amp *= persistance;
            }

            return result;
        }

		float ridgeOctaveNoise(float x, float y, std::int32_t octaves, int ridgeStartingOctave, float lacunarity = 2.f, float persistance = 0.25f) const
		{
			float result1 = 0.0f;
            float result2 = 0.f;
			float amp = 1.0f;
			std::int32_t i = 0;
            if (ridgeStartingOctave > 0)
            {
                for (; i < ridgeStartingOctave; ++i)
                {
                    result1 += noise(x, y) * amp;
                    x *= lacunarity;
                    y *= lacunarity;
                    amp *= persistance;
                }
                result1 = (result1 * 0.5f + 0.5f);
            }

            for (; i < octaves; ++i)
			{
				result2 += noise(x, y) * amp;
				x *= lacunarity;
				y *= lacunarity;
				amp *= persistance;
			}
            result2 = std::abs(result2);
			float ret = result1 + result2;
			if (ret >= 1.f)
			{
				ret = 1.f - (ret - 1.f);
			}
			if (ret <= 0.f)
			{
				ret = 0.f;
			}
			return ret;
		}

        float octaveNoise(float x, float y, float z, std::int32_t octaves, float lacunarity = 2.f, float persistance = 0.25f) const
        {
            float result = 0.0f;
            float amp = 1.0f;

            for (std::int32_t i = 0; i < octaves; ++i)
            {
                result += noise(x, y, z) * amp;
                x *= lacunarity;
                y *= lacunarity;
                z *= lacunarity;
                amp *= persistance;
            }

            return result;
        }

        float noise0_1(float x) const
        {
            return noise(x) * 0.5f + 0.5f;
        }

        float noise0_1(float x, float y) const
        {
            return noise(x, y) * 0.5f + 0.5f;
        }

        float noise0_1(float x, float y, float z) const
        {
            return noise(x, y, z) * 0.5f + 0.5f;
        }

        float octaveNoise0_1(float x, std::int32_t octaves, float lacunarity = 2.f, float persistance = 0.25f) const
        {
            return octaveNoise(x, octaves, lacunarity, persistance) * 0.5f + 0.5f;
        }

        float octaveNoise0_1(float x, float y, std::int32_t octaves, float lacunarity = 2.f, float persistance = 0.25f) const
        {
           return octaveNoise(x, y, octaves, lacunarity, persistance) * 0.5f + 0.5f;
        }

		float ridgedNoise0_1(float x, float y, std::int32_t octaves, int ridgeStartingOctave = 1, float lacunarity = 2.f, float persistance = 0.25f) const
		{
			return ridgeOctaveNoise(x, y, octaves, ridgeStartingOctave, lacunarity, persistance);
		}

        float octaveNoise0_1(float x, float y, float z, std::int32_t octaves, float lacunarity = 2.f, float persistance = 0.25f) const
        {
            return octaveNoise(x, y, z, octaves, lacunarity, persistance) * 0.5f + 0.5f;
        }

		//https://www.classes.cs.uchicago.edu/archive/2015/fall/23700-1/final-project/MusgraveTerrain00.pdf

		   /* Procedural fBm evaluated at "point"; returns value stored in "value".
			*
			* Parameters:
		    *"H" is the fractal increment
			* "lacunarity" is the gap between successive frequencies
			* "octaves" is the number of frequencies in the fBm
			* "Basis()" is usually Perlin noise
			*/

		std::function<double(double, double)> Basis = [=](double x, double y) { return (double)this->noise0_1((float)x, (float)y); };

		double fBm(double x, double y, double H, double lacunarity, int32_t octaves) const
		{
#define MAX_OCTAVES 17
			double value;
			static bool first = true;
			static double lastH, lastLacunarity;
			static double exponent_array[MAX_OCTAVES];
			int i;

			/* precompute and store spectral weights */
			if (first || lastH != H || lastLacunarity != lacunarity)
			{
				double frequency = 1.0;
				for (i = 0; i<MAX_OCTAVES; i++) {
					/* compute weight for each frequency */
					exponent_array[i] = pow(frequency, -H);
					frequency *= lacunarity;
				}
				first = false;
				lastH = H;
				lastLacunarity = lacunarity;
			}
			value = 0.0;
			/* inner loop of spectral construction */
			for (i = 0; i<octaves; i++) {
				value += Basis(x,y) * exponent_array[i];
				x *= lacunarity;
				y *= lacunarity;
			} /* for */
			double remainder = octaves - (int)octaves;
			if (remainder) /* add in "octaves" remainder */
						   /* "i" and spatial freq. are preset in loop above */
				value += remainder * Basis(x,y) * exponent_array[i];
			return(value / 3.0);
		} /* fBm() */


		  /* Domain-distorted fBm.
		  *
		  * Some good parameter values to start with:
		  *
		  * H: 0.25
		  * distortion: 0.3
		  */
		double WarpedFBm(double x, double y, double H = 0.25, double lacunarity = 2.0, int32_t octaves = 8, double distortion = 0.3) const
		{
			double Noise3();
			double tmpX, tmpY, distortX, distortY;
			/* compute distortion vector */
			tmpX = x;
			tmpY = y;
			distortX = fBm(tmpX, tmpY, H, lacunarity, octaves);
			tmpX += 10.5;
			tmpY += 10.5;
			distortY = fBm(tmpX, tmpY, H, lacunarity, octaves);

			/* add distortion to sample point */
			x += distortion * distortX;
			y += distortion * distortY;
			return(fBm(x, y, H, lacunarity, octaves));
		} /* WarpedFBm() */
    };
}