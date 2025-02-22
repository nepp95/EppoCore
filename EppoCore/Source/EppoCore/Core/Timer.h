#pragma once

#include <chrono>

namespace Eppo
{
	class Timer
	{
	public:
		Timer()
		{
			m_startPoint = std::chrono::high_resolution_clock::now();
		}

		void Reset()
		{
			m_startPoint = std::chrono::high_resolution_clock::now();
		}

		[[nodiscard]] float GetElapsedMilliseconds() const
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_startPoint).count() * 0.001f * 0.001f * 0.001f;
		}

		[[nodiscard]] float GetElapsedMicroseconds() const
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_startPoint).count() * 0.001f * 0.001f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_startPoint;
	};
}
