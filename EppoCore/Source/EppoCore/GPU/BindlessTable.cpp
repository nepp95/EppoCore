#include "pch.h"
#include "BindlessTable.h"

#include "EppoCore/GPU/PhysicalDevice.h"

namespace Eppo
{
    BindlessTable::BindlessTable(VkDevice device, const PhysicalDevice& physicalDevice, uint32_t maxImages, uint32_t maxSamplers)
        : m_Device(device)
    {
        ZoneScoped;

        VkPhysicalDeviceVulkan12Properties vulkan12Properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
        VkPhysicalDeviceProperties2 properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &vulkan12Properties };
        vkGetPhysicalDeviceProperties2(physicalDevice.GetHandle(), &properties);

        maxImages = std::min(maxImages, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages);
        maxSamplers = std::min(maxSamplers, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers);

        const std::array bindings = {
            VkDescriptorSetLayoutBinding{
                                         .binding = ImageBinding,
                                         .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                         .descriptorCount = maxImages,
                                         .stageFlags = VK_SHADER_STAGE_ALL,
                                         },
            VkDescriptorSetLayoutBinding{
                                         .binding = SamplerBinding,
                                         .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                                         .descriptorCount = maxSamplers,
                                         .stageFlags = VK_SHADER_STAGE_ALL,
                                         },
        };

        // Slots may be empty, and may be written while the set is bound or in use by pending work (for other slots)
        constexpr VkDescriptorBindingFlags bindingFlag = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
        constexpr std::array bindingFlags = { bindingFlag, bindingFlag };

        const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data(),
        };

        const VkDescriptorSetLayoutCreateInfo layoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &bindingFlagsInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
        };

        EP_VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_Layout));

        const std::array poolSizes = {
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxImages   },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,       maxSamplers },
        };

        const VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        EP_VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_Pool));

        const VkDescriptorSetAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_Pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_Layout,
        };

        EP_VK_CHECK(vkAllocateDescriptorSets(m_Device, &allocateInfo, &m_Set));

        // Hand out the lowest indices first
        for (uint32_t i = maxImages; i > 0; i--)
            m_FreeImages.push_back(i - 1);
        for (uint32_t i = maxSamplers; i > 0; i--)
            m_FreeSamplers.push_back(i - 1);
    }

    BindlessTable::~BindlessTable()
    {
        vkDestroyDescriptorPool(m_Device, m_Pool, nullptr);
        vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
    }

    auto BindlessTable::AddImage(const VkImageView view) -> uint32_t
    {
        if (m_FreeImages.empty())
        {
            Log::Error("The bindless image table is full");
            return InvalidIndex;
        }

        const uint32_t index = m_FreeImages.back();
        m_FreeImages.pop_back();

        Write(
            ImageBinding, index, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            { .imageView = view, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
        );
        return index;
    }

    auto BindlessTable::AddSampler(const VkSampler sampler) -> uint32_t
    {
        if (m_FreeSamplers.empty())
        {
            Log::Error("The bindless sampler table is full");
            return InvalidIndex;
        }

        const uint32_t index = m_FreeSamplers.back();
        m_FreeSamplers.pop_back();

        Write(SamplerBinding, index, VK_DESCRIPTOR_TYPE_SAMPLER, { .sampler = sampler });
        return index;
    }

    auto BindlessTable::RemoveImage(const uint32_t index) -> void
    {
        if (index != InvalidIndex)
            m_FreeImages.push_back(index);
    }

    auto BindlessTable::RemoveSampler(const uint32_t index) -> void
    {
        if (index != InvalidIndex)
            m_FreeSamplers.push_back(index);
    }

    auto
    BindlessTable::Write(const uint32_t binding, const uint32_t index, const VkDescriptorType type, const VkDescriptorImageInfo& info) const
        -> void
    {
        const VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_Set,
            .dstBinding = binding,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = type,
            .pImageInfo = &info,
        };

        vkUpdateDescriptorSets(m_Device, 1, &write, 0, nullptr);
    }
}
