#include "Timer.h"
#include "SDL/include/SDL_timer.h"


Timer::Timer()
{
	Start();
}

void Timer::Start()
{
	m_startedAt = SDL_GetTicks();
}


unsigned __int32 Timer::ReadMs() const
{
	return SDL_GetTicks() - m_startedAt;
}

float Timer::ReadMsPrecise() const
{
	return SDL_GetTicks() - m_startedAt;
}

unsigned __int64 PerfTimer::m_frequency = 0;

PerfTimer::PerfTimer()
{
	if (m_frequency == 0)
		m_frequency = SDL_GetPerformanceFrequency();
	Start();
}


void PerfTimer::Start()
{
	m_startedAt = SDL_GetPerformanceCounter();
}


unsigned int PerfTimer::ReadMs() const
{
	return 1000.0 * (double(SDL_GetPerformanceCounter() - m_startedAt) / double(m_frequency));
}

float PerfTimer::ReadMsPrecise() const
{
	return 1000.0 * (double(SDL_GetPerformanceCounter() - m_startedAt) / double(m_frequency));
}
