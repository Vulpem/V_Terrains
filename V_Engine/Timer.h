#ifndef __TIMER__
#define __TIMER__

class TimerInterface
{
public:
	virtual void Start() = 0;
	virtual unsigned int ReadMs() const = 0;
	virtual float ReadMsPrecise() const = 0;
};

class Timer : public TimerInterface
{
public:
	Timer();

	void Start() override;
	unsigned int ReadMs() const override;
	float ReadMsPrecise() const override;

private:
	unsigned __int32 m_startedAt;
};

class PerfTimer : public TimerInterface
{
public:
	PerfTimer();

	void Start() override;
	unsigned int ReadMs() const override;
	float ReadMsPrecise() const override;

private:
	unsigned __int64 m_startedAt;
	static unsigned __int64 m_frequency;
};

#endif //__TIMER_H__