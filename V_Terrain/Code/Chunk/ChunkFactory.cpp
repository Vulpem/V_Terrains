//  RPG Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hern�ndez L�zaro
//  
//  "RPG Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "RPG Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with RPG Terrains.  If not, see <http://www.gnu.org/licenses/>.
#include "ChunkFactory.h"
#include <thread>
#include <chrono>

#include "../Noise/PerlinNoise.h"

namespace RPGT
{
    ChunkFactory::ChunkFactory()
		: m_runningThread(false)
		, m_wantToStopThread(false)
		, m_mut_requests()
		, m_mut_results()
		, m_mut_wantToStopThread()
		, m_thread()
    {
        LaunchThread();
    }

	ChunkFactory::~ChunkFactory()
	{
		StopThread();
		m_thread.join();
	}

	void ChunkFactory::LaunchThread()
	{
		std::unique_lock<std::mutex> lock(m_mut_wantToStopThread);
		if (m_runningThread == false)
		{
			m_thread = std::thread(&ChunkFactory::ThreadLoop, this);
			m_runningThread = true;
		}
	}

	void ChunkFactory::StopThread()
	{
		std::unique_lock<std::mutex> lock(m_mut_wantToStopThread);
		m_wantToStopThread = true;
	}

    void ChunkFactory::ClearRequests()
    {
		std::unique_lock<std::mutex> lock(m_mut_requests);
        m_requests.clear();
    }

    bool ChunkFactory::IsRequested(Vec2<int> p)
    {
        auto it = std::find(m_requests.begin(), m_requests.end(), p);
        if (it != m_requests.end())
        {
            return true;
        }
        return false;
    }

    void ChunkFactory::SetSeed(uint seed)
    {
        std::unique_lock<std::mutex> curveLock(m_mut_noiseGenerator);
        m_noiseGenerator.SetSeed(seed);
    }

    void ChunkFactory::PushChunkRequest(RequestedChunk request)
    {
		std::unique_lock<std::mutex> lock(m_mut_requests);
        if (IsRequested(request.pos) == false)
        {
            m_requests.push_back(request);
        }
    }

    GeneratedChunk ChunkFactory::PopGeneratedChunk()
    {
		std::unique_lock<std::mutex> lock(m_mut_results);
        const GeneratedChunk ret = m_results.front();
        m_results.pop();
        return ret;
    }

    bool ChunkFactory::HasGeneratedChunks()
    {
		std::unique_lock<std::mutex> lock(m_mut_results);
        return (!m_results.empty());
    }

    RequestedChunk ChunkFactory::PopChunkRequest()
    {
		std::unique_lock<std::mutex> lock(m_mut_requests);
        const RequestedChunk ret = *m_requests.begin();
        m_requests.erase(m_requests.begin());
        return ret;
    }

    void ChunkFactory::PushGeneratedChunk(const GeneratedChunk & generated)
    {
		std::unique_lock<std::mutex> lock(m_mut_results);
        m_results.push(generated);
    }

    bool ChunkFactory::HasRequestedChunks()
    {
		std::unique_lock<std::mutex> lock(m_mut_requests);
        return (!m_requests.empty());
    }

    void ChunkFactory::GenerateChunk()
    {
        if (HasRequestedChunks())
        {
            RequestedChunk request = PopChunkRequest();
            GeneratedChunk result;
            RPGT::NoiseMap noiseMap;
            {
                std::unique_lock<std::mutex> curveLock(m_mut_noiseGenerator);
                noiseMap = m_noiseGenerator.GenNoiseMap(request.pos);
            }

            result.m_pos = request.pos;
            result.m_size = { noiseMap.Width() - 2, noiseMap.Height() - 2 };
            result.m_data.resize(result.m_size.x() * result.m_size.y() * 4);

            const float topLeftX = (noiseMap.Width() - (noiseMap.Width() % 2 != 0)) / 2.f;
            const float topLeftY = (noiseMap.Height() - (noiseMap.Height() % 2 != 0)) / 2.f;

            uint current = 0;

            for (uint y = noiseMap.Height() - 2; y >= 1 ; y--)
            {
                for (uint x = noiseMap.Width() - 2; x >= 1 ; x--)
                {
                    const Vec3<float> central(topLeftX - x, noiseMap[x + y * noiseMap.Width()], topLeftY - y);

                    const Vec3<float> top = central - Vec3<float>(topLeftX - x, noiseMap[x + (y + 1) * noiseMap.Width()], topLeftY - (y + 1));
                    const Vec3<float> bottom = central - Vec3<float>(topLeftX - x, noiseMap[x + (y - 1) * noiseMap.Width()], topLeftY - (y - 1));
                    const Vec3<float> right = central - Vec3<float>(topLeftX - x + 1, noiseMap[x + 1 + y * noiseMap.Width()], topLeftY - y);
                    const Vec3<float> left = central - Vec3<float>(topLeftX - x - 1, noiseMap[x - 1 + y * noiseMap.Width()], topLeftY - y);

					Vec3<float> norm =
						top.Cross(left)
						+ left.Cross(bottom)
						+ bottom.Cross(right)
                        + right.Cross(top);

                    norm.Normalize();

                    result[current++] = norm.x() * 0.5f + 0.5f;
                    result[current++] = norm.y() * 0.5f + 0.5f;
                    result[current++] = norm.z() * 0.5f + 0.5f;

                    result[current++] = noiseMap[x + y * noiseMap.Width()];
                }
            }
            PushGeneratedChunk(result);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
    }

    void ChunkFactory::ThreadLoop()
    {
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_mut_wantToStopThread);
				if (m_wantToStopThread)
				{
					m_runningThread = false;
					return;
				}
			}

			GenerateChunk();
		}
    }
}