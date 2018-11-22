#include "TimerManager.h"

TimerManager::TimerManager()
{
}

TimerManager::~TimerManager()
{
	for (auto timer : m_timers)
	{
		delete timer.second;
	}
}

void TimerManager::CreatePerfTimer(std::string key)
{
	const uint id = m_timerIDs.size();
	m_timerIDs.insert(std::pair<std::string, uint>(key, id));

	std::pair<uint, TimerInterface*> timer(id, new PerfTimer());
	m_timers.insert(timer);

	m_storedValues.insert(std::pair<uint, std::pair<std::string, float>>(id, std::pair<std::string, float>(key, 0.0f)));
}

void TimerManager::CreateTimer(std::string key)
{
	uint id = m_timerIDs.size();
	m_timerIDs.insert(std::pair<std::string, uint>(key, id));

	std::pair<uint, TimerInterface*> timer(id, new Timer());
	m_timers.insert(timer);

	m_storedValues.insert(std::pair<uint, std::pair<std::string, float>>(id, std::pair<std::string, float>(key, 0.0f)));
}

void TimerManager::StartTimer(std::string key)
{
	std::map<std::string, uint>::iterator timerID = m_timerIDs.find(key);
	if (timerID != m_timerIDs.end())
	{
		m_timers[timerID->second]->Start();
	}
	else
	{
		CreateTimer(key);
	}
}

void TimerManager::StartTimerPerf(std::string key)
{
	std::map<std::string, uint>::iterator timerID = m_timerIDs.find(key);
	if (timerID != m_timerIDs.end())
	{
		m_timers[timerID->second]->Start();
	}
	else
	{
		CreatePerfTimer(key);
	}
}


void TimerManager::ResetTimerStoredVal(std::string key)
{
	std::map<std::string, uint>::iterator IDit = m_timerIDs.find(key);
	if (IDit != m_timerIDs.end())
	{
		m_storedValues.find(IDit->second)->second.second = 0.0f;
	}
}

float TimerManager::ReadMsStoredVal(std::string key)
{
	std::map<std::string, uint>::iterator IDit = m_timerIDs.find(key);
	if (IDit != m_timerIDs.end())
	{
		return m_storedValues.find(IDit->second)->second.second;
	}
}

float TimerManager::ReadMs(std::string key)
{
	std::map<std::string, uint>::iterator timerID = m_timerIDs.find(key);
	if (timerID != m_timerIDs.end())
	{
		const float time = m_timers[timerID->second]->ReadMsPrecise();
		m_storedValues[timerID->second].second = time;
		return time;
	}
	LOG("Tried to read the timer %s, which is unexistant", key.data());
	return 0.0f;
}

float TimerManager::ReadMS_Max(std::string key)
{
	std::map<std::string, uint>::iterator timerID = m_timerIDs.find(key);
	if (timerID != m_timerIDs.end())
	{
		float timerCurrentValue = m_timers[timerID->second]->ReadMsPrecise();
		auto storedValue = m_storedValues[timerID->second];
		storedValue.second = MAX(storedValue.second, timerCurrentValue);
		return storedValue.second;
	}
	LOG("Tried to read the timer %s, which is unexistant", key.data());
	return 0.0f;
}

uint TimerManager::ReadSec(std::string key)
{
	return ReadMs(key) / 1000;
}

std::string TimerManager::GetKeyFromID(uint id)
{
	for (std::map<std::string, uint>::iterator it = m_timerIDs.begin(); it != m_timerIDs.end(); it++)
	{
		if (it->second == id)
		{
			return it->first;
		}
	}
	return std::string();
}

std::vector<std::pair<std::string, float>> TimerManager::GetLastReads()
{
	std::vector<std::pair<std::string, float>> ret;
		for (auto timer : m_timerIDs)
		{
			ret.push_back(m_storedValues[timer.second]);
		}
	return ret;
}
