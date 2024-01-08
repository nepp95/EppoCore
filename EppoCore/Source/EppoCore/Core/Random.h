#pragma once

#include <glm/glm.hpp>

#include <random>

namespace Eppo
{
	class FastRandom
	{
	public:
		// PCG Hash: https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
		// Returns an integer between 0 and UINT32_MAX
		static uint32_t PCG_Hash(uint32_t input)
		{
			uint32_t state = input * 747796405u + 2891336453u;
			uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
			return (word >> 22u) ^ word;
		}

		// Returns a float between 0 and 1
		static float Float(uint32_t& seed)
		{
			seed = PCG_Hash(seed);
			return (float)seed / (float)UINT32_MAX;
		}

		static glm::vec3 InUnitSphere(uint32_t& seed)
		{
			return glm::normalize(
				glm::vec3(Float(seed) * 2.0f - 1.0f, Float(seed) * 2.0f - 1.0f, Float(seed) * 2.0f - 1.0f)
			);
		}
	};

	class Random
	{
	public:
		static int32_t Int32(int32_t min = INT32_MIN, int32_t max = INT32_MAX)
		{
			std::uniform_int_distribution<int32_t> distribution(min, max);
			return distribution(s_Engine32);
		}

		static int64_t Int64(int64_t min = INT64_MIN, int64_t max = INT64_MAX)
		{
			std::uniform_int_distribution<int64_t> distribution(min, max);
			return distribution(s_Engine64);
		}

		static uint32_t UInt32(uint32_t min = 0, uint32_t max = UINT32_MAX)
		{
			std::uniform_int_distribution<uint32_t> distribution(min, max);
			return distribution(s_Engine32);
		}

		static uint64_t UInt64(uint64_t min = 0, uint64_t max = UINT64_MAX)
		{
			std::uniform_int_distribution<uint64_t> distribution(min, max);
			return distribution(s_Engine64);
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

	private:
		inline static std::mt19937_64 s_Engine64;
		inline static std::mt19937 s_Engine32;
	};
}
