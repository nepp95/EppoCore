#include "pch.h"
#include "Vulkan.h"

#include <vulkan/vk_enum_string_helper.h>

namespace Eppo
{
    auto VkResultToString(const VkResult result) -> const char*
    {
        return string_VkResult(result);
    }

    auto VkFatal(const std::string_view message) -> void
    {
        Log::Error("{}", message);
        EP_ASSERT(false);
        std::abort();
    }
}
