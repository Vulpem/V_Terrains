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
#pragma once

#include "../Globals.h"

#include "../Noise/PerlinNoise.h"
#include "ChunkRequests.h"

#include <mutex>
#include <thread>

namespace RPGT
{
    class ChunkFactory
    {
    public:
		ChunkFactory();
		~ChunkFactory();

        ChunkFactory(const ChunkFactory&) = delete;
        ChunkFactory& operator=(const ChunkFactory&) = delete;

		void LaunchThread();
		bool IsThreadRunning() const { return m_runningThread; }
		void StopThread();

        void ClearRequests();
        void SetHeightCurve(std::function<float(float)> func);
        void SetSeed(uint seed);

        void PushChunkRequest(RequestedChunk request);
        GeneratedChunk PopGeneratedChunk();
        bool HasGeneratedChunks();

    private:
        bool IsRequested(Vec2<int> p);

        RequestedChunk PopChunkRequest();
        void PushGeneratedChunk(const GeneratedChunk& generated);
		bool HasRequestedChunks();

		void ThreadLoop();
        void GenerateChunk();

		PerlinNoise m_noiseGenerator;
        std::mutex m_mut_noiseGenerator;

        std::list<RequestedChunk> m_requests;
		std::mutex m_mut_requests;
        std::queue<GeneratedChunk> m_results;
		std::mutex m_mut_results;

		bool m_runningThread;
		bool m_wantToStopThread;
		std::mutex m_mut_wantToStopThread;

        std::thread m_thread;
    };
}