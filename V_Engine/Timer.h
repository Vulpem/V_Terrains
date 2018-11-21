#ifndef __TIMER_H__
#define __TIMER_H__

#include "Globals.h"
#include "SDL\include\SDL.h"

class Timer
{
public:
	Timer();

	void Start();
	Uint32 ReadMs();

private:
	bool	m_running;
	Uint32	m_startedAt;
};

#endif //__TIMER_H__