#include "pch.h"
#include "PhysicalDevice.h"

#include "EppoCore/GPU/Instance.h"

namespace Eppo
{
    namespace
    {
        constexpr VkSubgroupFeatureFlags s_RequiredSubgroupOperations = VK_SUBGROUP_FEATURE_BASIC_BIT | VK_SUBGROUP_FEATURE_VOTE_BIT |
            VK_SUBGROUP_FEATURE_ARITHMETIC_BIT | VK_SUBGROUP_FEATURE_BALLOT_BIT | VK_SUBGROUP_FEATURE_SHUFFLE_BIT |
            VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT;

        constexpr std::array s_RequiredExtensions = { VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME };

        // Enables every required feature in enabled; returns the names of the ones supported doesn't have
        auto RequireFeatures(const FeatureChain& supported, FeatureChain& enabled) -> std::vector<std::string>
        {
            std::vector<std::string> missing;

#define EP_REQUIRE_FEATURE(structure, feature)                                                                                             \
    if (supported.structure.feature)                                                                                                       \
        enabled.structure.feature = VK_TRUE;                                                                                               \
    else                                                                                                                                   \
        missing.emplace_back(#feature)

            // 16-bit floats
            EP_REQUIRE_FEATURE(Vulkan11, storageBuffer16BitAccess);
            EP_REQUIRE_FEATURE(Vulkan12, shaderFloat16);

            // Timeline semaphores, buffer device address, host query reset
            EP_REQUIRE_FEATURE(Vulkan12, timelineSemaphore);
            EP_REQUIRE_FEATURE(Vulkan12, bufferDeviceAddress);
            EP_REQUIRE_FEATURE(Vulkan12, hostQueryReset);

            // Descriptor indexing (bindless table)
            EP_REQUIRE_FEATURE(Vulkan12, descriptorIndexing);
            EP_REQUIRE_FEATURE(Vulkan12, runtimeDescriptorArray);
            EP_REQUIRE_FEATURE(Vulkan12, descriptorBindingPartiallyBound);
            EP_REQUIRE_FEATURE(Vulkan12, descriptorBindingSampledImageUpdateAfterBind);
            EP_REQUIRE_FEATURE(Vulkan12, descriptorBindingUpdateUnusedWhilePending);
            EP_REQUIRE_FEATURE(Vulkan12, shaderSampledImageArrayNonUniformIndexing);

            // Dynamic rendering, synchronization2
            EP_REQUIRE_FEATURE(Vulkan13, dynamicRendering);
            EP_REQUIRE_FEATURE(Vulkan13, synchronization2);

#undef EP_REQUIRE_FEATURE

            // Optional
            enabled.Features.features.samplerAnisotropy = supported.Features.features.samplerAnisotropy;
            enabled.Vulkan11.uniformAndStorageBuffer16BitAccess = supported.Vulkan11.uniformAndStorageBuffer16BitAccess;

            return missing;
        }

        auto QueryExtensions(const VkPhysicalDevice physicalDevice) -> std::vector<VkExtensionProperties>
        {
            uint32_t count = 0;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data());
            return extensions;
        }

        auto HasExtension(const std::vector<VkExtensionProperties>& extensions, const std::string_view name) -> bool
        {
            return std::ranges::any_of(
                extensions,
                [name](const VkExtensionProperties& extension)
                {
                    return name == extension.extensionName;
                }
            );
        }

        auto GetQueueFamilyProperties(const VkPhysicalDevice physicalDevice) -> std::vector<VkQueueFamilyProperties>
        {
            uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
            std::vector<VkQueueFamilyProperties> families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, families.data());
            return families;
        }

        // First family with all of the required flags and none of the excluded ones
        auto FindQueueFamily(const std::vector<VkQueueFamilyProperties>& families, const VkQueueFlags required, const VkQueueFlags excluded)
            -> std::optional<uint32_t>
        {
            for (uint32_t i = 0; i < families.size(); i++)
            {
                const VkQueueFlags flags = families[i].queueFlags;
                if ((flags & required) == required && (flags & excluded) == 0)
                    return i;
            }

            return std::nullopt;
        }
    }

    FeatureChain::FeatureChain()
    {
        Features.pNext = &Vulkan11;
        Vulkan11.pNext = &Vulkan12;
        Vulkan12.pNext = &Vulkan13;
    }

    auto PhysicalDevice::Select(const Instance& instance, const std::vector<const char*>& extraExtensions) -> PhysicalDevice
    {
        ZoneScoped;

        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance.GetHandle(), &count, nullptr);
        std::vector<VkPhysicalDevice> candidates(count);
        vkEnumeratePhysicalDevices(instance.GetHandle(), &count, candidates.data());

        PhysicalDevice selected;
        int bestScore = -1;

        for (const VkPhysicalDevice candidate : candidates)
        {
            VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProperties{
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR
            };
            VkPhysicalDeviceVulkan11Properties vulkan11Properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
                                                                   &pushDescriptorProperties };
            VkPhysicalDeviceProperties2 properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &vulkan11Properties };
            vkGetPhysicalDeviceProperties2(candidate, &properties);

            std::vector<std::string> missing;

            if (properties.properties.apiVersion < VK_API_VERSION_1_3)
                missing.emplace_back("Vulkan 1.3");

            const auto extensions = QueryExtensions(candidate);
            for (const char* extension : s_RequiredExtensions)
            {
                if (!HasExtension(extensions, extension))
                    missing.emplace_back(extension);
            }

            for (const char* extension : extraExtensions)
            {
                if (!HasExtension(extensions, extension))
                    missing.emplace_back(extension);
            }

            FeatureChain supported;
            FeatureChain enabled;
            vkGetPhysicalDeviceFeatures2(candidate, &supported.Features);
            for (auto& feature : RequireFeatures(supported, enabled))
                missing.push_back(std::move(feature));

            if (!(vulkan11Properties.subgroupSupportedStages & VK_SHADER_STAGE_COMPUTE_BIT) ||
                (vulkan11Properties.subgroupSupportedOperations & s_RequiredSubgroupOperations) != s_RequiredSubgroupOperations)
                missing.emplace_back("subgroup operations in compute shaders");

            const auto families = GetQueueFamilyProperties(candidate);
            if (!FindQueueFamily(families, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0))
                missing.emplace_back("a graphics + compute queue");

            if (!missing.empty())
            {
                std::string list;
                for (const auto& item : missing)
                    list += (list.empty() ? "" : ", ") + item;

                Log::Warn("Skipping GPU {}, missing: {}", properties.properties.deviceName, list);
                continue;
            }

            int score = 0;
            if (properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                score = 2;
            else if (properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                score = 1;

            if (score <= bestScore)
                continue;

            bestScore = score;
            selected.m_PhysicalDevice = candidate;
            selected.m_Properties = properties.properties;
            selected.m_SubgroupSize = vulkan11Properties.subgroupSize;
            selected.m_MaxPushDescriptors = pushDescriptorProperties.maxPushDescriptors;
            selected.m_QueueFamilyProperties = families;

            selected.m_Extensions.assign(s_RequiredExtensions.begin(), s_RequiredExtensions.end());
            selected.m_Extensions.insert(selected.m_Extensions.end(), extraExtensions.begin(), extraExtensions.end());
            selected.m_HasCalibratedTimestamps = HasExtension(extensions, VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
            if (selected.m_HasCalibratedTimestamps)
                selected.m_Extensions.push_back(VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
        }

        if (!selected.m_PhysicalDevice)
            VkFatal("No GPU supports everything EppoCore requires");

        // Queue families
        auto& queueFamilies = selected.m_QueueFamilies;
        const auto& families = selected.m_QueueFamilyProperties;
        queueFamilies.Graphics = *FindQueueFamily(families, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0);
        queueFamilies.Compute = FindQueueFamily(families, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT).value_or(queueFamilies.Graphics);
        queueFamilies.Transfer =
            FindQueueFamily(families, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT).value_or(queueFamilies.Compute);

        const auto& properties = selected.m_Properties;
        Log::Info(
            "Using GPU {} (Vulkan {}.{}.{}, subgroup size {})", properties.deviceName, VK_API_VERSION_MAJOR(properties.apiVersion),
            VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion), selected.m_SubgroupSize
        );

        return selected;
    }

    auto PhysicalDevice::EnableFeatures(FeatureChain& enabled) const -> void
    {
        FeatureChain supported;
        vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &supported.Features);
        RequireFeatures(supported, enabled);
    }

    auto PhysicalDevice::GetTimestampValidBits(const uint32_t queueFamily) const -> uint32_t
    {
        return queueFamily < m_QueueFamilyProperties.size() ? m_QueueFamilyProperties[queueFamily].timestampValidBits : 0;
    }
}
