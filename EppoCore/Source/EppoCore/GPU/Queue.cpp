#include "pch.h"
#include "Queue.h"

#include "EppoCore/GPU/Instance.h"
#include "EppoCore/GPU/PhysicalDevice.h"

namespace Eppo
{
    SubmitHandle::SubmitHandle(const Queue* queue, const uint64_t value)
        : m_Queue(queue), m_Value(value)
    {
    }

    auto SubmitHandle::Wait() const -> void
    {
        if (m_Queue)
            m_Queue->Wait(m_Value);
    }

    auto SubmitHandle::IsDone() const -> bool
    {
        return !m_Queue || m_Queue->GetCompletedValue() >= m_Value;
    }

    Queue::Queue(
        const QueueType type, const uint32_t family, const VkDevice device, const PhysicalDevice& physicalDevice, const Instance& instance
    )
        : m_Type(type), m_Family(family), m_Device(device)
    {
        ZoneScoped;

        vkGetDeviceQueue(m_Device, m_Family, 0, &m_Queue);

        const VkSemaphoreTypeCreateInfo timelineInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0,
        };

        const VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineInfo,
        };

        EP_VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Timeline));

        const VkCommandPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_Family,
        };

        EP_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool));

        // Tracy GPU context; some (transfer) queue families can't write timestamps
        if (physicalDevice.GetTimestampValidBits(m_Family) == 0)
            return;

        if (physicalDevice.HasCalibratedTimestamps())
        {
            [[maybe_unused]] const auto getTimeDomains = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(
                vkGetInstanceProcAddr(instance.GetHandle(), "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR")
            );
            [[maybe_unused]] const auto getTimestamps =
                reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(vkGetDeviceProcAddr(m_Device, "vkGetCalibratedTimestampsKHR"));

            m_TracyContext =
                TracyVkContextHostCalibrated(physicalDevice.GetHandle(), m_Device, vkResetQueryPool, getTimeDomains, getTimestamps);
        }
        else
        {
            // The uncalibrated context records and submits its initial timestamp itself
            const VkCommandBufferAllocateInfo allocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = m_CommandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
            EP_VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocateInfo, &commandBuffer));
            m_TracyContext = TracyVkContext(physicalDevice.GetHandle(), m_Device, m_Queue, commandBuffer);
            vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
        }

        if (m_TracyContext)
        {
            constexpr std::array<std::string_view, QueueTypeCount> names = { "Graphics queue", "Compute queue", "Transfer queue" };
            [[maybe_unused]] const std::string_view name = names[static_cast<size_t>(m_Type)];
            TracyVkContextName(m_TracyContext, name.data(), static_cast<uint16_t>(name.size()));
        }
    }

    Queue::~Queue()
    {
        if (m_TracyContext)
            TracyVkDestroy(m_TracyContext);

        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        vkDestroySemaphore(m_Device, m_Timeline, nullptr);
    }

    auto Queue::Submit(
        const VkCommandBuffer commandBuffer, const std::span<const SemaphoreSubmit> waits, const std::span<const SemaphoreSubmit> signals
    ) -> SubmitHandle
    {
        ZoneScoped;

        const uint64_t value = ++m_SubmittedValue;

        const VkCommandBufferSubmitInfo commandBufferInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = commandBuffer,
        };

        std::vector<VkSemaphoreSubmitInfo> waitInfos;
        for (const SemaphoreSubmit& wait : waits)
            waitInfos.push_back(
                { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = wait.Semaphore, .stageMask = wait.Stages }
            );

        // The timeline semaphore first, then the extra binary ones
        std::vector<VkSemaphoreSubmitInfo> signalInfos = {
            { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
             .semaphore = m_Timeline,
             .value = value,
             .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT },
        };
        for (const SemaphoreSubmit& signal : signals)
            signalInfos.push_back(
                { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = signal.Semaphore, .stageMask = signal.Stages }
            );

        const VkSubmitInfo2 submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = static_cast<uint32_t>(waitInfos.size()),
            .pWaitSemaphoreInfos = waitInfos.data(),
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &commandBufferInfo,
            .signalSemaphoreInfoCount = static_cast<uint32_t>(signalInfos.size()),
            .pSignalSemaphoreInfos = signalInfos.data(),
        };

        EP_VK_CHECK(vkQueueSubmit2(m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
        return { this, value };
    }

    auto Queue::Wait(const uint64_t value) const -> void
    {
        ZoneScoped;

        const VkSemaphoreWaitInfo waitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1,
            .pSemaphores = &m_Timeline,
            .pValues = &value,
        };

        EP_VK_CHECK(vkWaitSemaphores(m_Device, &waitInfo, UINT64_MAX));
    }

    auto Queue::GetCompletedValue() const -> uint64_t
    {
        uint64_t value = 0;
        EP_VK_CHECK(vkGetSemaphoreCounterValue(m_Device, m_Timeline, &value));
        return value;
    }

    auto Queue::CollectProfilingData() -> void
    {
        if (m_TracyContext)
            TracyVkCollect(m_TracyContext, VK_NULL_HANDLE);
    }
}
