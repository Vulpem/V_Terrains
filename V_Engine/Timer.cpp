// ----------------------------------------------------
// Timer.cpp
// Body for CPU Tick Timer class
// ----------------------------------------------------

#include "Timer.h"
#include "SDL/include/SDL_timer.h"

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
unsigned __int32 Timer::ReadMs()
{
	return SDL_GetTicks() - m_startedAt;
}


