#include "pch.h"
#include "PipelineCache.h"

#include "EppoCore/Core/Filesystem.h"

namespace Eppo
{
    PipelineCache::PipelineCache(const VkDevice device, std::filesystem::path path)
        : m_Device(device), m_Path(std::move(path))
    {
        ZoneScoped;

        // The driver ignores data from another GPU/driver (or corrupt data) and starts empty
        Buffer data;
        if (!m_Path.empty() && std::filesystem::exists(m_Path))
            data = ReadBytes(m_Path);

        const VkPipelineCacheCreateInfo cacheInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .initialDataSize = data.Size,
            .pInitialData = data.Data,
        };

        EP_VK_CHECK(vkCreatePipelineCache(m_Device, &cacheInfo, nullptr, &m_Cache));
        data.Release();
    }

    PipelineCache::~PipelineCache()
    {
        Save();
        vkDestroyPipelineCache(m_Device, m_Cache, nullptr);
    }

    auto PipelineCache::Save() const -> void
    {
        ZoneScoped;

        if (m_Path.empty())
            return;

        size_t size = 0;
        EP_VK_CHECK(vkGetPipelineCacheData(m_Device, m_Cache, &size, nullptr));
        std::vector<char> data(size);
        EP_VK_CHECK(vkGetPipelineCacheData(m_Device, m_Cache, &size, data.data()));

        std::error_code error;
        if (m_Path.has_parent_path())
            std::filesystem::create_directories(m_Path.parent_path(), error);

        std::ofstream out(m_Path, std::ios::binary);
        if (!out)
        {
            Log::Warn("Could not save the pipeline cache to {}", m_Path);
            return;
        }

        out.write(data.data(), static_cast<std::streamsize>(size));
    }
}
