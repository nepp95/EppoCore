#pragma once

typedef unsigned int GLuint;

namespace Eppo
{
	class ComputeShader
	{
	public:
		ComputeShader(const std::string& filepath);

		void Bind() const;
		void Dispatch(uint32_t x, uint32_t y, uint32_t z) const;

		// TODO: This should not be in this class, but we currently have no other use for it
		void MemBarrier() const;



	private:
		void CheckCompileErrors(GLuint shader, std::string type);

	private:
		uint32_t m_RendererID;
	};
}
