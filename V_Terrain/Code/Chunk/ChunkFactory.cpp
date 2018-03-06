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
#include "ChunkFactory.h"
#include "../Noise/PerlinNoise.h"

#include <math.h>

namespace VTerrain
{
    ChunkFactory::ChunkFactory()
    {
    }

    void ChunkFactory::EmptyQueue()
    {
        m_requests.empty();
    }

    bool ChunkFactory::IsRequested(Vec2<int> p)
    {
        for (auto it : m_requests)
        {
            if ((it) == p)
            {
                return true;
            }
        }
        return false;
    }

    void ChunkFactory::PushChunkRequest(RequestedChunk request)
    {
        m_requests.push_back(request);
    }

    ChunkFactory::GeneratedChunk ChunkFactory::PopGeneratedChunk()
    {
        const GeneratedChunk ret = m_results.front();
        m_results.pop();
        return ret;
    }

    bool ChunkFactory::HasGeneratedChunks()
    {
        return (!m_results.empty());
    }

    ChunkFactory::RequestedChunk ChunkFactory::PopChunkRequest()
    {
        const RequestedChunk ret = *m_requests.begin();
        m_requests.erase(m_requests.begin());
        return ret;
    }

    void ChunkFactory::PushGeneratedChunk(const GeneratedChunk & generated)
    {
        m_results.push(generated);
    }

    bool ChunkFactory::HasRequestedChunks()
    {
        return (!m_requests.empty());
    }

    void ChunkFactory::GenerateChunk()
    {
        if (HasRequestedChunks())
        {
            RequestedChunk request = PopChunkRequest();
            GeneratedChunk result;
            VTerrain::PerlinNoise::NoiseMap noiseMap = VTerrain::PerlinNoise::GenNoiseMap(request.pos);

            result.m_pos = request.pos;
            result.m_size = { noiseMap.Width() - 2, noiseMap.Height() - 2 };
            result.m_LOD = 0;
            result.m_data.resize(result.m_size.x() * result.m_size.y() * 4);

            const float topLeftX = (noiseMap.Width() - (noiseMap.Width() % 2 != 0)) / 2.f;
            const float topLeftY = (noiseMap.Height() - (noiseMap.Height() % 2 != 0)) / 2.f;

            uint current = 0;

            for (uint y = 1; y < result.m_size.y() - 1; y++)
            {
                for (uint x = 1; x < result.m_size.x() - 1; x++)
                {
                    const Vec3<float> central(topLeftX - x, noiseMap[x + y * noiseMap.Width()] * Config::maxHeight, topLeftY - y);
                    const Vec3<float> top = central - Vec3<float>(topLeftX - x, noiseMap[x + (y + 1) * noiseMap.Width()] * Config::maxHeight, topLeftY - (y + 1));
                    const Vec3<float> bottom = central - Vec3<float>(topLeftX - x, noiseMap[x + (y - 1) * noiseMap.Width()] * Config::maxHeight, topLeftY - (y - 1));
                    const Vec3<float> right = central - Vec3<float>(topLeftX - x + 1, noiseMap[x + 1 + y * noiseMap.Width()] * Config::maxHeight, topLeftY - y);
                    const Vec3<float> left = central - Vec3<float>(topLeftX - x - 1, noiseMap[x - 1 + y * noiseMap.Width()] * Config::maxHeight, topLeftY - y);

                    Vec3<float> norm =
                        top.Cross(left)
                        + left.Cross(bottom)
                        + bottom.Cross(right)
                        + right.Cross(top);

                    norm.Normalize();

                    result[current + 0] = noiseMap[x + y * noiseMap.Width()];
                    result[current + 1] = norm.x();
                    result[current + 2] = norm.y();
                    result[current + 3] = norm.z();

                    current += 4;
                }
            }

            m_results.push(result);
        }
    }

    void ChunkFactory::ThreadLoop()
    {
        GenerateChunk();
    }
}