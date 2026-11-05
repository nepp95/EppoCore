#include "pch.h"
#include "Instance.h"

namespace Eppo
{
    namespace
    {
        constexpr std::string_view s_ValidationLayer = "VK_LAYER_KHRONOS_validation";

        VKAPI_ATTR auto VKAPI_CALL DebugCallback(
            const VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT,
            const VkDebugUtilsMessengerCallbackDataEXT* data, void*
        ) -> VkBool32
        {
            if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                Log::Error("Vulkan: {}", data->pMessage);
            else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                Log::Warn("Vulkan: {}", data->pMessage);
            else
                Log::Info("Vulkan: {}", data->pMessage);

            return VK_FALSE;
        }

        auto HasLayer(const std::string_view name) -> bool
        {
            uint32_t count = 0;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> layers(count);
            vkEnumerateInstanceLayerProperties(&count, layers.data());

            return std::ranges::any_of(
                layers,
                [name](const VkLayerProperties& layer)
                {
                    return name == layer.layerName;
                }
            );
        }

        auto HasExtension(const std::string_view name) -> bool
        {
            uint32_t count = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

            return std::ranges::any_of(
                extensions,
                [name](const VkExtensionProperties& extension)
                {
                    return name == extension.extensionName;
                }
            );
        }
    }

    Instance::Instance(const InstanceSpecification& specification)
    {
        ZoneScoped;

        uint32_t apiVersion = 0;
        EP_VK_CHECK(vkEnumerateInstanceVersion(&apiVersion));
        if (apiVersion < VK_API_VERSION_1_3)
            VkFatal(
                fmt::format(
                    "Vulkan 1.3 is required, but the Vulkan loader only supports {}.{}", VK_API_VERSION_MAJOR(apiVersion),
                    VK_API_VERSION_MINOR(apiVersion)
                )
            );

        std::vector<const char*> layers;
        if (specification.EnableValidation)
        {
            if (HasLayer(s_ValidationLayer))
                layers.push_back(s_ValidationLayer.data());
            else
                Log::Warn("Vulkan validation was requested, but {} is not installed", s_ValidationLayer);
        }

        std::vector<const char*> extensions = specification.Extensions;
        for (const char* extension : extensions)
        {
            if (!HasExtension(extension))
                VkFatal(fmt::format("Required Vulkan instance extension is not available: {}", extension));
        }

        const bool debugUtils = specification.EnableValidation && HasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (debugUtils)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        const VkApplicationInfo applicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "EppoCore",
            .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .pEngineName = "EppoCore",
            .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3,
        };

        const VkDebugUtilsMessengerCreateInfoEXT messengerInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugCallback,
        };

        const VkInstanceCreateInfo instanceInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            // Chaining the messenger also reports problems during vkCreateInstance/vkDestroyInstance
            .pNext = debugUtils ? &messengerInfo : nullptr,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        EP_VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_Instance));

        if (debugUtils)
        {
            const auto createMessenger =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));
            EP_VK_CHECK(createMessenger(m_Instance, &messengerInfo, nullptr, &m_DebugMessenger));

            m_SetDebugUtilsObjectName =
                reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(m_Instance, "vkSetDebugUtilsObjectNameEXT"));
        }
    }

    Instance::~Instance()
    {
        if (m_DebugMessenger)
        {
            const auto destroyMessenger =
                reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
            destroyMessenger(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
    }
}
