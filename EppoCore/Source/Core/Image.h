#pragma once

#include <glad/glad.h>

namespace Eppo
{
	class Image
	{
	public:
		Image(uint32_t width, uint32_t height);
		~Image();

		uint32_t GetRendererID() const { return m_RendererID; }
		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		void SetData(void* data, uint32_t size);
		void Bind(uint32_t slot = 0);

	private:
		uint32_t m_RendererID;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;
	};
}
