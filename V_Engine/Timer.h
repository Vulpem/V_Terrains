#ifndef __TIMER_H__
#define __TIMER_H__

#include "Globals.h"
#include "SDL\include\SDL.h"

class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	void Stop();

	Uint32 Read();

private:

	bool	m_running;
	Uint32	m_startedAt;
	Uint32	m_stoppedAt;
};

#endif //__TIMER_H__