#pragma once

#include "EppoCore/Core/Base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Eppo
{
    class Log
    {
    public:
        static auto Init() -> void;

        template<typename... Args>
        static constexpr auto Trace(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            s_Logger->trace(fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static constexpr auto Info(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            s_Logger->info(fmt, std::forward<Args>(args)...);
        }
        template<typename... Args>
        static constexpr auto Warn(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            s_Logger->warn(fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static constexpr auto Error(fmt::format_string<Args...> fmt, Args&&... args) -> void
        {
            s_Logger->error(fmt, std::forward<Args>(args)...);
        }

    private:
        static std::shared_ptr<spdlog::logger> s_Logger;
    };
}

template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string_view>
{
    auto format(const std::filesystem::path& v, format_context& ctx) const -> format_context::iterator
    {
        return formatter<std::string_view>::format(v.string(), ctx);
    }
};
