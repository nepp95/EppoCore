#pragma once

#include <glm/glm.hpp>

namespace Eppo
{
	class Buffer
	{
	public:
		Buffer(uint32_t size);
		Buffer(void* data, uint32_t size);
		~Buffer();

		void Bind() const;
		
		glm::vec4* MapBuffer() const;
		void UnmapBuffer() const;

	private:
		uint32_t m_RendererID;
		uint32_t m_Size;
	};
}
