#include <chrono>
#include <Core/Log.hpp>

class ScopedTimer
{
public:
	ScopedTimer(std::string name)
		: m_name(name), m_startTime(std::chrono::high_resolution_clock::now())
	{
	}
	~ScopedTimer()
	{
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_startTime);
		CORE_INFO("{} took {}ms", m_name, duration.count());
	}
private:
	const std::string m_name;
	std::chrono::time_point<std::chrono::steady_clock>	m_startTime;
};


#define TIMER(x) { ScopedTimer(std::string(#x)); x();}