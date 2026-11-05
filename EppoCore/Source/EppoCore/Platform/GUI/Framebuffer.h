#pragma once

#include "EppoCore/GPU/Image.h"

namespace Eppo
{
    class Device;

    struct FramebufferSpecification
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        ImageFormat Format = ImageFormat::RGBA8;
    };

    // An offscreen color target that can be sampled, e.g. shown inside an ImGui window with ToImGuiTextureID
    class Framebuffer
    {
    public:
        Framebuffer(Device& device, FramebufferSpecification specification);
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        auto operator=(const Framebuffer&) -> Framebuffer& = delete;
        Framebuffer(Framebuffer&&) = delete;
        auto operator=(Framebuffer&&) -> Framebuffer& = delete;

        auto Resize(uint32_t width, uint32_t height) -> void;

        [[nodiscard]] auto IsValid() const -> bool { return m_ColorAttachment.has_value(); }
        [[nodiscard]] auto GetImage() const -> const Image& { return *m_ColorAttachment; }
        [[nodiscard]] auto GetSpecification() const -> const FramebufferSpecification& { return m_Specification; }
        [[nodiscard]] auto GetWidth() const -> uint32_t { return m_Specification.Width; }
        [[nodiscard]] auto GetHeight() const -> uint32_t { return m_Specification.Height; }

    private:
        Device* m_Device;
        FramebufferSpecification m_Specification;
        std::optional<Image> m_ColorAttachment;
    };
}
