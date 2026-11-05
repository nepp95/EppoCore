#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    class Instance;

    // Features2 -> Vulkan 1.1 -> 1.2 -> 1.3, linked on construction (so it can't be copied)
    struct FeatureChain
    {
        VkPhysicalDeviceFeatures2 Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        VkPhysicalDeviceVulkan11Features Vulkan11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
        VkPhysicalDeviceVulkan12Features Vulkan12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        VkPhysicalDeviceVulkan13Features Vulkan13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };

        FeatureChain();
        FeatureChain(const FeatureChain&) = delete;
        auto operator=(const FeatureChain&) -> FeatureChain& = delete;
    };

    struct QueueFamilies
    {
        uint32_t Graphics = 0; // graphics + compute
        uint32_t Compute = 0; // dedicated compute family when there is one, otherwise Graphics
        uint32_t Transfer = 0; // dedicated transfer family when there is one, otherwise Compute
    };

    // The GPU EppoCore runs on, and everything EppoCore requires from it
    class PhysicalDevice
    {
    public:
        // Picks the best GPU (discrete first) that supports everything EppoCore requires, plus the extra device extensions
        [[nodiscard]] static auto Select(const Instance& instance, const std::vector<const char*>& extraExtensions) -> PhysicalDevice;

        // Enables every feature EppoCore requires (all supported, Select guarantees it) plus the optional ones the GPU has
        auto EnableFeatures(FeatureChain& enabled) const -> void;

        // EppoCore's required extensions, the optional ones the GPU has, and the extra ones passed to Select
        [[nodiscard]] auto GetExtensions() const -> const std::vector<const char*>& { return m_Extensions; }
        [[nodiscard]] auto HasCalibratedTimestamps() const -> bool { return m_HasCalibratedTimestamps; }

        [[nodiscard]] auto GetHandle() const -> VkPhysicalDevice { return m_PhysicalDevice; }
        [[nodiscard]] auto GetProperties() const -> const VkPhysicalDeviceProperties& { return m_Properties; }
        [[nodiscard]] auto GetSubgroupSize() const -> uint32_t { return m_SubgroupSize; }
        // Most descriptors one push descriptor set (set 0) can hold
        [[nodiscard]] auto GetMaxPushDescriptors() const -> uint32_t { return m_MaxPushDescriptors; }
        [[nodiscard]] auto GetQueueFamilies() const -> const QueueFamilies& { return m_QueueFamilies; }
        [[nodiscard]] auto GetTimestampValidBits(uint32_t queueFamily) const -> uint32_t;

    private:
        PhysicalDevice() = default;

    private:
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_Properties{};
        uint32_t m_SubgroupSize = 0;
        uint32_t m_MaxPushDescriptors = 0;
        QueueFamilies m_QueueFamilies;
        std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
        std::vector<const char*> m_Extensions;
        bool m_HasCalibratedTimestamps = false;
    };
}
