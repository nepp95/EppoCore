#include "pch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Eppo
{
	std::shared_ptr<spdlog::logger> Log::s_Logger;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T][%l]: %v%$");

		s_Logger = spdlog::stdout_color_mt("EppoRays");
		s_Logger->set_level(spdlog::level::trace);
	}
}
