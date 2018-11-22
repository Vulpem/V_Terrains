#ifndef __TIMERMANAGER__
#define __TIMERMANAGER__

#include "Globals.h"
#include "Timer.h"

class TimerManager
{
public:
	TimerManager();
	~TimerManager();

	void CreatePerfTimer(std::string key);
	void CreateTimer(std::string key);
	void StartTimer(std::string key);
	void StartTimerPerf(std::string key);
	void ResetTimerStoredVal(std::string key);
	float ReadMsStoredVal(std::string key);
	float ReadMs(std::string key);
	float ReadMS_Max(std::string key);
	uint ReadSec(std::string key);
	std::string GetKeyFromID(uint id);
	std::vector<std::pair<std::string, float>> GetLastReads();

private:
	std::map<uint, TimerInterface*> m_timers;

	//Map that associates each ID with its key and stored value
	std::map<uint, std::pair<std::string, float>> m_storedValues;

	//Map that associates each key with its ID
	std::map<std::string, uint> m_timerIDs;
};

#endif // !__TIMERMANAGER__
