#pragma once

#include "Core/Base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <Spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Eppo
{
	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetLogger() { return s_Logger; }

	private:
		static Ref<spdlog::logger> s_Logger;
	};
}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternio)
{
	return os << glm::to_string(quaternio);
}

#define EPPO_TRACE(...)			::Eppo::Log::GetLogger()->trace(__VA_ARGS__)
#define EPPO_INFO(...)			::Eppo::Log::GetLogger()->info(__VA_ARGS__)
#define EPPO_WARN(...)			::Eppo::Log::GetLogger()->warn(__VA_ARGS__)
#define EPPO_ERROR(...)			::Eppo::Log::GetLogger()->error(__VA_ARGS__)
#define EPPO_CRITICAL(...)		::Eppo::Log::GetLogger()->critical(__VA_ARGS__)
