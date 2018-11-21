#include "Globals.h"
#include "Application.h"
#include "Timers.h"

void CreatePerfTimer(std::string key)
{
	App->m_timers.CreatePerfTimer(key);
}

void CreateTimer(std::string key)
{
	App->m_timers.CreateTimer(key);
}

float ReadMs(std::string key)
{
	return App->m_timers.ReadMs(key);
}

unsigned int ReadSec(std::string key)
{
	return App->m_timers.ReadSec(key);
}

void StartTimer(std::string key)
{
	App->m_timers.StartTimer(key);
}

void StartTimerPerf(std::string key)
{
	App->m_timers.StartTimerPerf(key);
}

float ReadMs_Max(std::string key)
{
	return App->m_timers.ReadMS_Max(key);
}

void ResetTimerStoredVal(std::string key)
{
	App->m_timers.ResetTimerStoredVal(key);
}

float ReadMsStoredVal(std::string key)
{
	return App->m_timers.ReadMsStoredVal(key);
}