#ifndef __PERFTIMER__
#define __PERFTIMER__

#include "Globals.h"


class PerfTimer
{
public:

	// Constructor
	PerfTimer();

	void OnEnable();
	double ReadMs() const;
	uint64 ReadTicks() const;

private:
	uint64	m_startedAt;
	static uint64 frequency;
};

#endif //__j1PERFTIMER_H__