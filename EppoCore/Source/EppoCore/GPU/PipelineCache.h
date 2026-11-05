#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    // VkPipelineCache loaded from and saved to disk. An empty path keeps it in memory only.
    class PipelineCache
    {
    public:
        PipelineCache(VkDevice device, std::filesystem::path path);
        ~PipelineCache();

        PipelineCache(const PipelineCache&) = delete;
        auto operator=(const PipelineCache&) -> PipelineCache& = delete;
        PipelineCache(PipelineCache&&) = delete;
        auto operator=(PipelineCache&&) -> PipelineCache& = delete;

        auto Save() const -> void;

        [[nodiscard]] auto GetHandle() const -> VkPipelineCache { return m_Cache; }

    private:
        VkDevice m_Device;
        std::filesystem::path m_Path;
        VkPipelineCache m_Cache = VK_NULL_HANDLE;
    };
}
