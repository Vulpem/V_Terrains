// ----------------------------------------------------
// Timer.cpp
// Body for CPU Tick Timer class
// ----------------------------------------------------

#include "Timer.h"

// ---------------------------------------------
Timer::Timer()
{
	Start();
}

// ---------------------------------------------
void Timer::Start()
{
	m_running = true;
	m_startedAt = SDL_GetTicks();
}

// ---------------------------------------------
void Timer::Stop()
{
	m_running = false;
	m_stoppedAt = SDL_GetTicks();
}

// ---------------------------------------------
Uint32 Timer::Read()
{
	if(m_running == true)
	{
		return SDL_GetTicks() - m_startedAt;
	}
	else
	{
		return m_stoppedAt - m_startedAt;
	}
}


