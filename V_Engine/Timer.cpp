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
	m_startedAt = SDL_GetTicks();
}

// ---------------------------------------------
Uint32 Timer::ReadMs()
{
	return SDL_GetTicks() - m_startedAt;
}


