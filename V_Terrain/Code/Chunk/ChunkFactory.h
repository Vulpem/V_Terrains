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
#pragma once

#include "../Utils/Globals.h"

#include <mutex>
#include <thread>

namespace VTerrain
{
    class ChunkFactory
    {
    public:
        struct RequestedChunk
        {
            RequestedChunk(int x = 0, int y = 0) : pos(x, y) {}
            RequestedChunk(Vec2<int> p) : pos(p) {}
            Vec2<int> pos;
            bool operator== (const RequestedChunk& p) { return pos == p.pos; }
            bool operator== (const Vec2<int>& p) { return pos == p; }
        };

        struct GeneratedChunk
        {
			GeneratedChunk() = default;
            Vec2<int> m_pos;
            Vec2<uint> m_size;
            uint m_LOD;
            std::vector<float> m_data;
            float& operator[] (int n) { return m_data[n]; }
        };

		ChunkFactory();
		~ChunkFactory();

		ChunkFactory(const ChunkFactory&) {};
        ChunkFactory& operator=(const ChunkFactory&) {};

		void LaunchThread();
		void StopThread();

        void EmptyQueue();
        bool IsRequested(Vec2<int> p);

        void PushChunkRequest(RequestedChunk request);
        GeneratedChunk PopGeneratedChunk();
        bool HasGeneratedChunks();
    private:
        RequestedChunk PopChunkRequest();
        void PushGeneratedChunk(const GeneratedChunk& generated);
        bool HasRequestedChunks();
        
		void ThreadLoop();
        void GenerateChunk();

        std::list<RequestedChunk> m_requests;
		std::mutex m_mut_requests;
        std::queue<GeneratedChunk> m_results;
		std::mutex m_mut_results;

		std::thread m_thread;
		bool m_runningThread;
		bool m_wantToStopThread;
		std::mutex m_mut_wantToStopThread;
    };
}