#ifndef __PERFTIMER__
#define __PERFTIMER__

class PerfTimer
{
public:
	PerfTimer();

	void Start();
	double ReadMs() const;

private:
	unsigned __int64 m_startedAt;
	static unsigned __int64 frequency;
};

#endif //__PERFTIMER_H__