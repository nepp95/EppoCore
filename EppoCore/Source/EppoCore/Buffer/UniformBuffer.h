#pragma once

namespace Eppo
{
	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		~UniformBuffer();

		uint32_t GetSize() const { return m_Size; }

		void SetData(void* data, uint32_t size);

	private:
		void Recreate();

	private:
		uint32_t m_RendererID;
		uint32_t m_Size;
		uint32_t m_Binding;
	};
}
