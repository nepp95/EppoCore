#include "pch.h"
#include "Sampler.h"

#include "EppoCore/GPU/BindlessTable.h"
#include "EppoCore/GPU/Device.h"

namespace Eppo
{
    namespace
    {
        auto ToVkFilter(const SamplerFilter filter) -> VkFilter
        {
            return filter == SamplerFilter::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        }

        auto ToVkAddressMode(const SamplerAddressMode mode) -> VkSamplerAddressMode
        {
            switch (mode)
            {
                case SamplerAddressMode::Repeat:
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                case SamplerAddressMode::MirroredRepeat:
                    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                case SamplerAddressMode::ClampToEdge:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            }

            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }
    }

    Sampler::Sampler(Device& device, SamplerSpecification specification)
        : m_Device(&device), m_Specification(std::move(specification))
    {
        ZoneScoped;

        const VkFilter filter = ToVkFilter(m_Specification.Filter);
        const VkSamplerAddressMode addressMode = ToVkAddressMode(m_Specification.AddressMode);

        const VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = filter,
            .minFilter = filter,
            .mipmapMode = m_Specification.Filter == SamplerFilter::Nearest ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = addressMode,
            .addressModeV = addressMode,
            .addressModeW = addressMode,
            .maxLod = VK_LOD_CLAMP_NONE,
        };

        EP_VK_CHECK(vkCreateSampler(m_Device->GetHandle(), &samplerInfo, nullptr, &m_Sampler));
        m_Device->SetDebugName(VK_OBJECT_TYPE_SAMPLER, VkHandleToU64(m_Sampler), m_Specification.DebugName);

        m_BindlessIndex = m_Device->AddBindlessSampler(m_Sampler);
    }

    Sampler::~Sampler()
    {
        Release();
    }

    Sampler::Sampler(Sampler&& other) noexcept
        : m_Device(other.m_Device),
          m_Specification(std::move(other.m_Specification)),
          m_Sampler(std::exchange(other.m_Sampler, VK_NULL_HANDLE)),
          m_BindlessIndex(std::exchange(other.m_BindlessIndex, BindlessTable::InvalidIndex))
    {
    }

    auto Sampler::operator=(Sampler&& other) noexcept -> Sampler&
    {
        if (this != &other)
        {
            Release();
            m_Device = other.m_Device;
            m_Specification = std::move(other.m_Specification);
            m_Sampler = std::exchange(other.m_Sampler, VK_NULL_HANDLE);
            m_BindlessIndex = std::exchange(other.m_BindlessIndex, BindlessTable::InvalidIndex);
        }

        return *this;
    }

    auto Sampler::Release() -> void
    {
        if (!m_Sampler)
            return;

        m_Device->RemoveBindlessSampler(m_BindlessIndex);
        m_Device->Defer(
            [device = m_Device->GetHandle(), sampler = m_Sampler]
            {
                vkDestroySampler(device, sampler, nullptr);
            }
        );

        m_Sampler = VK_NULL_HANDLE;
        m_BindlessIndex = BindlessTable::InvalidIndex;
    }
}
