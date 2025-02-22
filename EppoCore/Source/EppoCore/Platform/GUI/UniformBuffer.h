#pragma once

namespace Eppo
{
    class UniformBuffer
    {
    public:
        UniformBuffer(uint32_t size, uint32_t binding);
        ~UniformBuffer();

        void SetData(const void* data, uint32_t size) const;

    private:
        uint32_t m_RendererID = 0;
    };
}
