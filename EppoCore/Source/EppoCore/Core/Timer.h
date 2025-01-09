#pragma once

#include <chrono>

namespace Eppo 
{
	class Timer
	{
	public:
		Timer()
		{
			m_startPoint = std::chrono::steady_clock::now();
		}

		[[nodiscard]] uint64_t GetElapsedMilliseconds() const
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_startPoint).count();
		}

		[[nodiscard]] uint64_t GetElapsedMicroseconds() const
		{
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_startPoint).count();
		}

	private:
		std::chrono::steady_clock::time_point m_startPoint;
	};
}
