#pragma once

namespace Eppo
{
    struct FramebufferSpecification
    {
        uint32_t Width;
        uint32_t Height;
    };

    class Framebuffer
    {
    public:
        explicit Framebuffer(FramebufferSpecification spec);
        ~Framebuffer();

        [[nodiscard]] uint32_t GetRendererID() const { return m_TextureID; }

        void Bind() const;
        static void Unbind();

        [[nodiscard]] const FramebufferSpecification& GetSpecification() const { return m_Specification; }

    private:
        FramebufferSpecification m_Specification;
        uint32_t m_FramebufferID = 0;
        uint32_t m_TextureID = 0;
    };
}
