#include "pch.h"
#include "Log.h"

namespace Eppo
{
    std::shared_ptr<spdlog::logger> Log::s_Logger = nullptr;

    void Log::Init()
    {
        spdlog::set_pattern("%^[%T][%l]: %v%$");

        s_Logger = spdlog::stdout_color_mt("EppoCore");
        s_Logger->set_level(spdlog::level::trace);

        spdlog::set_default_logger(s_Logger);
    }
}
