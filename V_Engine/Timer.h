#ifndef __TIMER__
#define __TIMER__

class Timer
{
public:
	Timer();

	void Start();
	unsigned int ReadMs();

private:
	bool m_running;
	unsigned __int32 m_startedAt;
};

#endif //__TIMER_H__