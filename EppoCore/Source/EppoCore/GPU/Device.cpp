#include "pch.h"
#include "Device.h"

#include "EppoCore/GPU/BindlessTable.h"
#include "EppoCore/GPU/CommandList.h"
#include "EppoCore/GPU/Instance.h"
#include "EppoCore/GPU/PipelineCache.h"

#include <vma/vk_mem_alloc.h>

namespace Eppo
{
    Device::Device(const DeviceSpecification& specification)
        : m_Instance(std::make_unique<Instance>(InstanceSpecification{ specification.EnableValidation, specification.InstanceExtensions })),
          m_PhysicalDevice(PhysicalDevice::Select(*m_Instance, specification.DeviceExtensions))
    {
        ZoneScoped;

        CreateDevice();
        CreateAllocator();

        m_BindlessTable =
            std::make_unique<BindlessTable>(m_Device, m_PhysicalDevice, specification.MaxBindlessImages, specification.MaxBindlessSamplers);
        SetDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET, VkHandleToU64(m_BindlessTable->GetSet()), "Bindless table");

        m_PipelineCache = std::make_unique<PipelineCache>(m_Device, specification.PipelineCachePath);
    }

    Device::~Device()
    {
        ZoneScoped;

        WaitIdle();
        m_DeletionQueue.Flush();

        m_PipelineCache.reset();
        m_BindlessTable.reset();
        for (auto& queue : m_Queues)
            queue.reset();

        vmaDestroyAllocator(m_Allocator);
        vkDestroyDevice(m_Device, nullptr);
        m_Instance.reset();
    }

    auto Device::Submit(CommandList& commandList) -> SubmitHandle
    {
        ZoneScoped;

        EP_ASSERT(&commandList.GetDevice() == this);
        EP_ASSERT(!commandList.IsSubmitted());

        if (commandList.IsRecording())
            commandList.End();

        const SubmitHandle handle = GetQueue(commandList.GetQueueType()).Submit(commandList.GetHandle());
        commandList.MarkSubmitted();

        CollectGarbage();
        return handle;
    }

    auto Device::WaitIdle() const -> void
    {
        ZoneScoped;

        EP_VK_CHECK(vkDeviceWaitIdle(m_Device));
    }

    auto Device::Defer(std::function<void()> destroy) -> void
    {
        m_DeletionQueue.Push(GetSubmittedValues(), std::move(destroy));
    }

    auto Device::CollectGarbage() -> void
    {
        ZoneScoped;

        QueueValues completed{};
        for (size_t i = 0; i < QueueTypeCount; i++)
        {
            completed[i] = m_Queues[i]->GetCompletedValue();
            m_Queues[i]->CollectProfilingData();
        }

        m_DeletionQueue.Collect(completed);
    }

    auto Device::AddBindlessImage(const VkImageView view) -> uint32_t
    {
        return m_BindlessTable->AddImage(view);
    }

    auto Device::AddBindlessSampler(const VkSampler sampler) -> uint32_t
    {
        return m_BindlessTable->AddSampler(sampler);
    }

    auto Device::RemoveBindlessImage(const uint32_t index) -> void
    {
        Defer(
            [table = m_BindlessTable.get(), index]
            {
                table->RemoveImage(index);
            }
        );
    }

    auto Device::RemoveBindlessSampler(const uint32_t index) -> void
    {
        Defer(
            [table = m_BindlessTable.get(), index]
            {
                table->RemoveSampler(index);
            }
        );
    }

    auto Device::SetDebugName(const VkObjectType type, const uint64_t handle, const std::string_view name) const -> void
    {
        const auto setName = m_Instance->GetSetDebugNameFunction();
        if (!setName || name.empty())
            return;

        const std::string nameString(name);

        const VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = type,
            .objectHandle = handle,
            .pObjectName = nameString.c_str(),
        };

        setName(m_Device, &nameInfo);
    }

    auto Device::CreateDevice() -> void
    {
        ZoneScoped;

        const QueueFamilies& families = m_PhysicalDevice.GetQueueFamilies();
        for (const uint32_t family : { families.Graphics, families.Compute, families.Transfer })
        {
            if (std::ranges::find(m_QueueFamilies, family) == m_QueueFamilies.end())
                m_QueueFamilies.push_back(family);
        }

        constexpr float priority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        for (const uint32_t family : m_QueueFamilies)
        {
            queueInfos.push_back(
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = family,
                    .queueCount = 1,
                    .pQueuePriorities = &priority,
                }
            );
        }

        FeatureChain features;
        m_PhysicalDevice.EnableFeatures(features);

        const auto& extensions = m_PhysicalDevice.GetExtensions();

        const VkDeviceCreateInfo deviceInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features.Features,
            .queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
            .pQueueCreateInfos = queueInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        EP_VK_CHECK(vkCreateDevice(m_PhysicalDevice.GetHandle(), &deviceInfo, nullptr, &m_Device));

        m_CmdPushDescriptorSet =
            reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(m_Device, "vkCmdPushDescriptorSetKHR"));

        const std::array queueFamilies = { families.Graphics, families.Compute, families.Transfer };
        constexpr std::array<std::string_view, QueueTypeCount> names = { "Graphics", "Compute", "Transfer" };

        for (size_t i = 0; i < QueueTypeCount; i++)
        {
            auto& queue = m_Queues[i];
            queue = std::make_unique<Queue>(static_cast<QueueType>(i), queueFamilies[i], m_Device, m_PhysicalDevice, *m_Instance);

            const std::string name(names[i]);
            SetDebugName(VK_OBJECT_TYPE_SEMAPHORE, VkHandleToU64(queue->GetTimeline()), name + " timeline");
            SetDebugName(VK_OBJECT_TYPE_COMMAND_POOL, VkHandleToU64(queue->GetCommandPool()), name + " command pool");
        }
    }

    auto Device::CreateAllocator() -> void
    {
        ZoneScoped;

        const VmaAllocatorCreateInfo allocatorInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = m_PhysicalDevice.GetHandle(),
            .device = m_Device,
            .instance = m_Instance->GetHandle(),
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };

        EP_VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
    }

    auto Device::GetSubmittedValues() const -> QueueValues
    {
        QueueValues values{};
        for (size_t i = 0; i < QueueTypeCount; i++)
            values[i] = m_Queues[i]->GetSubmittedValue();
        return values;
    }
}
