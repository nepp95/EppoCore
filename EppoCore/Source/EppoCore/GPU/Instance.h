#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    struct InstanceSpecification
    {
        bool EnableValidation = false;
        // Extra extensions, e.g. the window's surface extensions
        std::vector<const char*> Extensions;
    };

    // The Vulkan instance, with the validation layer and debug messenger when validation is enabled
    class Instance
    {
    public:
        explicit Instance(const InstanceSpecification& specification);
        ~Instance();

        Instance(const Instance&) = delete;
        auto operator=(const Instance&) -> Instance& = delete;
        Instance(Instance&&) = delete;
        auto operator=(Instance&&) -> Instance& = delete;

        [[nodiscard]] auto GetHandle() const -> VkInstance { return m_Instance; }

        // Nullptr when debug utils aren't enabled
        [[nodiscard]] auto GetSetDebugNameFunction() const -> PFN_vkSetDebugUtilsObjectNameEXT { return m_SetDebugUtilsObjectName; }

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        PFN_vkSetDebugUtilsObjectNameEXT m_SetDebugUtilsObjectName = nullptr;
    };
}
