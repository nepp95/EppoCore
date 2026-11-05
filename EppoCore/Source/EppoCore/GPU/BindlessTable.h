#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    class PhysicalDevice;

    // The global bindless descriptor set (set 1): binding 0 = texture2D[], binding 1 = sampler[].
    // GLSL declarations: #include <EppoCore/Bindless.glsl>
    class BindlessTable
    {
    public:
        static constexpr uint32_t Set = 1;
        static constexpr uint32_t ImageBinding = 0;
        static constexpr uint32_t SamplerBinding = 1;
        static constexpr uint32_t InvalidIndex = UINT32_MAX;

        // The sizes are clamped to what the GPU supports
        BindlessTable(VkDevice device, const PhysicalDevice& physicalDevice, uint32_t maxImages, uint32_t maxSamplers);
        ~BindlessTable();

        BindlessTable(const BindlessTable&) = delete;
        auto operator=(const BindlessTable&) -> BindlessTable& = delete;
        BindlessTable(BindlessTable&&) = delete;
        auto operator=(BindlessTable&&) -> BindlessTable& = delete;

        // The image is expected in SHADER_READ_ONLY_OPTIMAL whenever a shader samples it. InvalidIndex when the table is full.
        [[nodiscard]] auto AddImage(VkImageView view) -> uint32_t;
        [[nodiscard]] auto AddSampler(VkSampler sampler) -> uint32_t;

        // Makes the slot available again immediately; only call once no submitted work uses it anymore
        auto RemoveImage(uint32_t index) -> void;
        auto RemoveSampler(uint32_t index) -> void;

        [[nodiscard]] auto GetLayout() const -> VkDescriptorSetLayout { return m_Layout; }
        [[nodiscard]] auto GetSet() const -> VkDescriptorSet { return m_Set; }

    private:
        auto Write(uint32_t binding, uint32_t index, VkDescriptorType type, const VkDescriptorImageInfo& info) const -> void;

    private:
        VkDevice m_Device;
        VkDescriptorPool m_Pool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
        VkDescriptorSet m_Set = VK_NULL_HANDLE;
        std::vector<uint32_t> m_FreeImages;
        std::vector<uint32_t> m_FreeSamplers;
    };
}
