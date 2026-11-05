#pragma once

#include "EppoCore/GPU/Vulkan.h"

#include <tracy/TracyVulkan.hpp>

namespace Eppo
{
    class Instance;
    class PhysicalDevice;
    class Queue;

    enum class QueueType : uint8_t
    {
        Graphics = 0,
        Compute,
        Transfer
    };

    constexpr uint32_t QueueTypeCount = 3;

    // A binary semaphore a submission waits on or signals, e.g. swapchain acquire/present
    struct SemaphoreSubmit
    {
        VkSemaphore Semaphore = VK_NULL_HANDLE;
        VkPipelineStageFlags2 Stages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    };

    // Completion of one submission, tracked through its queue's timeline semaphore
    class SubmitHandle
    {
    public:
        SubmitHandle() = default;
        SubmitHandle(const Queue* queue, uint64_t value);

        auto Wait() const -> void;
        [[nodiscard]] auto IsDone() const -> bool;

        [[nodiscard]] auto GetValue() const -> uint64_t { return m_Value; }

    private:
        const Queue* m_Queue = nullptr;
        uint64_t m_Value = 0;
    };

    // A device queue with its own timeline semaphore, command pool and Tracy GPU context.
    // Several queue types share one VkQueue when the GPU has no dedicated family for them.
    class Queue
    {
    public:
        Queue(QueueType type, uint32_t family, VkDevice device, const PhysicalDevice& physicalDevice, const Instance& instance);
        ~Queue();

        Queue(const Queue&) = delete;
        auto operator=(const Queue&) -> Queue& = delete;
        Queue(Queue&&) = delete;
        auto operator=(Queue&&) -> Queue& = delete;

        // Submits a finished command buffer; it signals the next timeline value when done
        auto
        Submit(VkCommandBuffer commandBuffer, std::span<const SemaphoreSubmit> waits = {}, std::span<const SemaphoreSubmit> signals = {})
            -> SubmitHandle;
        auto Wait(uint64_t value) const -> void;
        [[nodiscard]] auto GetCompletedValue() const -> uint64_t;
        [[nodiscard]] auto GetSubmittedValue() const -> uint64_t { return m_SubmittedValue; }

        // Reads back finished GPU zones for Tracy
        auto CollectProfilingData() -> void;

        [[nodiscard]] auto GetType() const -> QueueType { return m_Type; }
        [[nodiscard]] auto GetHandle() const -> VkQueue { return m_Queue; }
        [[nodiscard]] auto GetFamily() const -> uint32_t { return m_Family; }
        [[nodiscard]] auto GetCommandPool() const -> VkCommandPool { return m_CommandPool; }
        [[nodiscard]] auto GetTimeline() const -> VkSemaphore { return m_Timeline; }
        // Nullptr when this queue family can't write timestamps
        [[nodiscard]] auto GetTracyContext() const -> TracyVkCtx { return m_TracyContext; }

    private:
        QueueType m_Type;
        uint32_t m_Family;
        VkDevice m_Device;
        VkQueue m_Queue = VK_NULL_HANDLE;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        VkSemaphore m_Timeline = VK_NULL_HANDLE;
        uint64_t m_SubmittedValue = 0;
        TracyVkCtx m_TracyContext = nullptr;
    };
}
