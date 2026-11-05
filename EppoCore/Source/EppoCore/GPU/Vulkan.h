#pragma once

#include "EppoCore/Core/Base.h"

#include <vulkan/vulkan.h>

#include <cstdlib>
#include <optional>

// Opaque VMA handles, so public headers don't need vk_mem_alloc.h
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

namespace Eppo
{
    [[nodiscard]] auto VkResultToString(VkResult result) -> const char*;

    // Logs and aborts; for problems EppoCore can't continue from (no usable GPU, missing Vulkan support)
    [[noreturn]] auto VkFatal(std::string_view message) -> void;

    // Vulkan handle -> uint64_t, for debug names
    template<typename T>
    auto VkHandleToU64(T handle) -> uint64_t
    {
        return reinterpret_cast<uint64_t>(handle);
    }
}

// Vulkan errors are unrecoverable here, so a failed call logs and aborts in every configuration
#define EP_VK_CHECK(expression)                                                                                                            \
    do                                                                                                                                     \
    {                                                                                                                                      \
        const VkResult epVkResult = (expression);                                                                                          \
        if (epVkResult != VK_SUCCESS)                                                                                                      \
            ::Eppo::VkFatal(std::string(#expression) + " failed: " + ::Eppo::VkResultToString(epVkResult));                                \
    }                                                                                                                                      \
    while (false)

// Bitwise operators for enum class flags
#define EP_ENUM_FLAGS(Enum)                                                                                                                \
    constexpr auto operator|(Enum a, Enum b)->Enum                                                                                         \
    {                                                                                                                                      \
        return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(a) | static_cast<std::underlying_type_t<Enum>>(b));             \
    }                                                                                                                                      \
    constexpr auto operator&(Enum a, Enum b)->Enum                                                                                         \
    {                                                                                                                                      \
        return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(a) & static_cast<std::underlying_type_t<Enum>>(b));             \
    }                                                                                                                                      \
    constexpr auto HasFlag(Enum value, Enum flag) -> bool                                                                                  \
    {                                                                                                                                      \
        return (value & flag) == flag;                                                                                                     \
    }
