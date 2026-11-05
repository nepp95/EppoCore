#include "pch.h"
#include "Shader.h"

namespace Eppo
{
    Shader::Shader()
    {
    }

    Shader::~Shader()
    {
    }

    void Shader::Bind() const
    {
    }

    uint32_t Shader::GetRendererID() const
    {
        return m_RendererID;
    }
}
