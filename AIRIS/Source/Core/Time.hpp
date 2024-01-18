class Time
{
public:
	// Only one class is allowed to start, update and do other
	friend class SApp;

	// PUBLIC FUNCTIONS
	static float Total() { return static_cast<float>(m_totalTime); }
	static float UnscaledTotal() { return static_cast<float>(m_unscaledTotalTime); }
	static float Delta() { return static_cast<float>(m_deltaTime); }
	static float UnscaledDelta() { return static_cast<float>(m_unscaledDeltaTime); }
	static float TimeScale() { return static_cast<float>(m_timeScale); }

	static float SetTimeScale(float scale) { m_timeScale = scale; }

private:
	// This class shouldn't be instantiated
	Time() = delete;
	// Should be called only once right before render/game loop
	static void Reset();
	// Should be called only once at the start of the frame
	static void Update();

private:
	static double m_secondsPerCount;
	static double m_deltaTime;
	static double m_unscaledDeltaTime;
	static double m_totalTime;
	static double m_unscaledTotalTime;

	static double m_timeScale;

	static long long m_startTime;
	static long long m_prevTime;
	static long long m_curTime;
};