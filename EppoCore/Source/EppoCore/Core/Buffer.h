#pragma once

#include <glm/glm.hpp>

namespace Eppo
{
	class Buffer
	{
	public:
		Buffer(uint32_t size, uint32_t binding);
		~Buffer();

		void SetData(void* data, uint32_t size);

		uint32_t GetSize() const { return m_Size; }

		glm::vec4* MapBuffer() const;
		void UnmapBuffer() const;

	private:
		uint32_t m_RendererID;
		uint32_t m_Size;
		uint32_t m_Binding;
	};
}
