#include "Time.hpp"
#include "Log.hpp"
// TODO: Abstract platform dependent headers and functions
#include <Windows.h>

// INTIALIZE STATIC ELEMENT
double Time::m_secondsPerCount = 0;
double Time::m_deltaTime = 0;
double Time::m_unscaledDeltaTime = 0;
double Time::m_totalTime = 0;
double Time::m_unscaledTotalTime = 0;

double Time::m_timeScale = 1;
long long Time::m_startTime = 0;
long long Time::m_prevTime = 0;
long long Time::m_curTime = 0;

void Time::Reset()
{
	long long countPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countPerSec);
	m_secondsPerCount = 1.0 / (double)countPerSec;

	// Start time
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	m_deltaTime = 0.;
	m_unscaledDeltaTime = 0.;
	m_timeScale = 1.;
	m_totalTime = 0;
	m_prevTime = m_startTime;
	m_curTime = m_startTime;
}

void Time::Update()
{
	// Update current time
	QueryPerformanceCounter((LARGE_INTEGER*)&m_curTime);

	// Time passed since last call to this function
	m_unscaledDeltaTime = (m_curTime - m_prevTime) * m_secondsPerCount;

	// If delta time is negative (TODO: look into this) force nonnegative
	m_unscaledDeltaTime *= (m_unscaledDeltaTime >= 0.0);
	m_deltaTime = m_unscaledDeltaTime * m_timeScale;

	// Update total time since the last time reset was called
	m_unscaledTotalTime = (m_curTime - m_startTime) * m_secondsPerCount;
	m_totalTime += m_deltaTime;

	// Prepare for next update
	m_prevTime = m_curTime;
}
