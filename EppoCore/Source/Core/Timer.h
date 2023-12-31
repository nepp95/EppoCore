#pragma once

#include <chrono>

using namespace std::chrono;

namespace Eppo 
{
	class Timer
	{
	public:
		Timer()
		{
			m_startPoint = steady_clock::now();
		}

		uint64_t GetElapsedMilliseconds() const
		{
			return duration_cast<milliseconds>(steady_clock::now() - m_startPoint).count();
		}

		uint64_t GetElapsedMicroseconds() const
		{
			return duration_cast<microseconds>(steady_clock::now() - m_startPoint).count();
		}

	private:
		steady_clock::time_point m_startPoint;
	};
}
