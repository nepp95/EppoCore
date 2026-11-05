#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    class Device;

    enum class SamplerFilter : uint8_t
    {
        Linear,
        Nearest,
    };

    enum class SamplerAddressMode : uint8_t
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
    };

    struct SamplerSpecification
    {
        SamplerFilter Filter = SamplerFilter::Linear;
        SamplerAddressMode AddressMode = SamplerAddressMode::ClampToEdge;
        std::string DebugName;
    };

    // A sampler, also added to the bindless table
    class Sampler
    {
    public:
        Sampler(Device& device, SamplerSpecification specification = {});
        ~Sampler();

        Sampler(const Sampler&) = delete;
        auto operator=(const Sampler&) -> Sampler& = delete;
        Sampler(Sampler&& other) noexcept;
        auto operator=(Sampler&& other) noexcept -> Sampler&;

        [[nodiscard]] auto GetHandle() const -> VkSampler { return m_Sampler; }
        [[nodiscard]] auto GetBindlessIndex() const -> uint32_t { return m_BindlessIndex; }
        [[nodiscard]] auto GetSpecification() const -> const SamplerSpecification& { return m_Specification; }

    private:
        auto Release() -> void;

    private:
        Device* m_Device = nullptr;
        SamplerSpecification m_Specification;
        VkSampler m_Sampler = VK_NULL_HANDLE;
        uint32_t m_BindlessIndex = UINT32_MAX;
    };
}
