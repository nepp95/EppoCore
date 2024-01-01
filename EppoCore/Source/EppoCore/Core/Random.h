#pragma once

#include <glm/glm.hpp>

#include <random>

namespace Eppo
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::mt19937 s_Engine32(s_RandomDevice());

	static std::uniform_real_distribution<float> s_UniformRealDistribution;

	namespace Random
	{
		static int32_t Int32(int32_t min = INT32_MIN, int32_t max = INT32_MAX)
		{
			std::uniform_int_distribution<int32_t> distribution(min, max);
			return distribution(s_Engine32);
		}

		static int64_t Int64(int64_t min = INT64_MIN, int64_t max = INT64_MAX)
		{
			std::uniform_int_distribution<int64_t> distribution(min, max);
			return distribution(s_Engine);
		}

		static uint32_t UInt32(uint32_t min = 0, uint32_t max = UINT32_MAX)
		{
			std::uniform_int_distribution<uint32_t> distribution(min, max);
			return distribution(s_Engine32);
		}

		static uint64_t UInt64(uint64_t min = 0, uint64_t max = UINT64_MAX)
		{
			std::uniform_int_distribution<uint64_t> distribution(min, max);
			return distribution(s_Engine);
		}

		static float Float(float min = FLT_MIN, float max = FLT_MAX)
		{
			std::uniform_real_distribution<float> distribution(min, max);
			return distribution(s_Engine32);
		}

		static glm::vec2 Vec2(float min = FLT_MIN, float max = FLT_MAX)
		{
			glm::vec2 result = glm::vec2(Float(min, max), Float(min, max));
			return result;
		}

		static glm::vec3 Vec3(float min = FLT_MIN, float max = FLT_MAX)
		{
			glm::vec3 result = glm::vec3(Float(min, max), Float(min, max), Float(min, max));
			return result;
		}

		static glm::vec4 Vec4(float min = FLT_MIN, float max = FLT_MAX)
		{
			glm::vec4 result = glm::vec4(Float(min, max), Float(min, max), Float(min, max), Float(min, max));
			return result;
		}
	};
}
