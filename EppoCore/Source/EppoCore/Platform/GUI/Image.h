#pragma once

typedef unsigned int GLenum;

namespace Eppo
{
	class Image
	{
	public:
		Image(uint32_t width, uint32_t height);
		~Image();

		[[nodiscard]] uint32_t GetRendererID() const { return m_RendererID; }
		[[nodiscard]] uint32_t GetWidth() const { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const { return m_Height; }

		void SetData(const void* data) const;
		void Bind(uint32_t slot = 0) const;

	private:
		uint32_t m_RendererID;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;
	};
}
