#pragma once

#include "EppoCore/GPU/DeletionQueue.h"
#include "EppoCore/GPU/PhysicalDevice.h"
#include "EppoCore/GPU/Queue.h"
#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    class BindlessTable;
    class CommandList;
    class Instance;
    class PipelineCache;

    struct DeviceSpecification
    {
#ifdef EP_DEBUG
        bool EnableValidation = true;
#else
        bool EnableValidation = false;
#endif

        // Extra extensions, e.g. the window's surface extensions (instance) and VK_KHR_swapchain (device)
        std::vector<const char*> InstanceExtensions;
        std::vector<const char*> DeviceExtensions;

        // Where the pipeline cache is loaded from and saved to; empty keeps it in memory only
        std::filesystem::path PipelineCachePath;

        // Size of the global bindless table (descriptor set 1), clamped to what the GPU supports
        uint32_t MaxBindlessImages = 4096;
        uint32_t MaxBindlessSamplers = 64;
    };

    // The logical device and its memory allocator, plus the instance, queues, bindless table, pipeline cache and deferred
    // destruction it's built from. Not thread-safe: use it (and everything created from it) from one thread.
    // Every resource created from a Device must be destroyed before the Device itself.
    class Device
    {
    public:
        explicit Device(const DeviceSpecification& specification = {});
        ~Device();

        Device(const Device&) = delete;
        auto operator=(const Device&) -> Device& = delete;
        Device(Device&&) = delete;
        auto operator=(Device&&) -> Device& = delete;

        // Ends the command list if it's still recording, and submits it to its queue
        // Optionally waits on / signals binary semaphores (e.g. swapchain acquire/present)
        auto Submit(CommandList& commandList, std::span<const SemaphoreSubmit> waits = {}, std::span<const SemaphoreSubmit> signals = {})
            -> SubmitHandle;
        auto WaitIdle() const -> void;

        // Runs destroy once all work submitted so far has finished on every queue
        auto Defer(std::function<void()> destroy) -> void;
        // Runs the deferred destructions whose work has finished and collects Tracy GPU zones; Submit calls this too
        auto CollectGarbage() -> void;

        // Bindless slots are only reused after the work submitted so far is done
        [[nodiscard]] auto AddBindlessImage(VkImageView view) -> uint32_t;
        [[nodiscard]] auto AddBindlessSampler(VkSampler sampler) -> uint32_t;
        auto RemoveBindlessImage(uint32_t index) -> void;
        auto RemoveBindlessSampler(uint32_t index) -> void;

        auto SetDebugName(VkObjectType type, uint64_t handle, std::string_view name) const -> void;

        [[nodiscard]] auto GetInstance() const -> const Instance& { return *m_Instance; }
        [[nodiscard]] auto GetPhysicalDevice() const -> const PhysicalDevice& { return m_PhysicalDevice; }
        [[nodiscard]] auto GetHandle() const -> VkDevice { return m_Device; }
        [[nodiscard]] auto GetAllocator() const -> VmaAllocator { return m_Allocator; }
        [[nodiscard]] auto GetQueue(QueueType type) -> Queue& { return *m_Queues[static_cast<size_t>(type)]; }
        [[nodiscard]] auto GetQueue(QueueType type) const -> const Queue& { return *m_Queues[static_cast<size_t>(type)]; }
        // The distinct queue families in use; resources are shared concurrently between them
        [[nodiscard]] auto GetQueueFamilies() const -> const std::vector<uint32_t>& { return m_QueueFamilies; }
        [[nodiscard]] auto GetBindlessTable() const -> const BindlessTable& { return *m_BindlessTable; }
        [[nodiscard]] auto GetPipelineCache() const -> const PipelineCache& { return *m_PipelineCache; }
        [[nodiscard]] auto GetPushDescriptorFunction() const -> PFN_vkCmdPushDescriptorSetKHR { return m_CmdPushDescriptorSet; }

    private:
        auto CreateDevice() -> void;
        auto CreateAllocator() -> void;
        [[nodiscard]] auto GetSubmittedValues() const -> QueueValues;

    private:
        std::unique_ptr<Instance> m_Instance;
        PhysicalDevice m_PhysicalDevice;
        VkDevice m_Device = VK_NULL_HANDLE;
        VmaAllocator m_Allocator = VK_NULL_HANDLE;

        std::array<std::unique_ptr<Queue>, QueueTypeCount> m_Queues;
        std::vector<uint32_t> m_QueueFamilies;

        std::unique_ptr<BindlessTable> m_BindlessTable;
        std::unique_ptr<PipelineCache> m_PipelineCache;
        DeletionQueue m_DeletionQueue;

        PFN_vkCmdPushDescriptorSetKHR m_CmdPushDescriptorSet = nullptr;
    };
}
