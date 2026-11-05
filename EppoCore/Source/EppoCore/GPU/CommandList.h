#pragma once

#include "EppoCore/GPU/Queue.h"
#include "EppoCore/GPU/Vulkan.h"

#include <glm/glm.hpp>

namespace Eppo
{
    class Device;
    class GpuBuffer;
    class Image;
    class Pipeline;
    class Sampler;

    // How a resource is used before or after a barrier. For images this also picks the layout:
    // Transfer* -> TRANSFER_SRC/DST, Compute* and Host* -> GENERAL, FragmentRead/ShaderRead -> SHADER_READ_ONLY,
    // ColorAttachment/DepthAttachment -> *_ATTACHMENT_OPTIMAL, Present -> PRESENT_SRC, None -> UNDEFINED (contents discarded).
    enum class Access : uint8_t
    {
        None,
        HostRead,
        HostWrite,
        TransferRead,
        TransferWrite,
        IndirectRead,
        VertexBuffer,
        IndexBuffer,
        UniformRead, // uniform buffers in any shader stage
        ComputeRead, // storage buffers/images read in compute shaders
        ComputeWrite, // storage buffers/images written in compute shaders
        FragmentRead, // sampled in fragment shaders
        ShaderRead, // read in any shader stage (sampled images: SHADER_READ_ONLY)
        ColorAttachment, // rendered to (read and write)
        DepthAttachment, // depth tested/written
        Present,
    };

    enum class IndexType : uint8_t
    {
        UInt16,
        UInt32,
    };

    enum class LoadOp : uint8_t
    {
        Load,
        Clear,
        DontCare,
    };

    struct ColorAttachment
    {
        const Image* Target = nullptr;
        LoadOp Load = LoadOp::Clear;
        glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct DepthAttachment
    {
        const Image* Target = nullptr;
        LoadOp Load = LoadOp::Clear;
        float ClearDepth = 1.0f;
    };

    struct RenderingSpecification
    {
        std::vector<ColorAttachment> ColorAttachments;
        DepthAttachment Depth;
    };

    // A command buffer for one queue, recording from construction until End() or Device::Submit. Single use: create a new
    // one per submission. Its command buffer is freed once the GPU is done with it.
    class CommandList
    {
    public:
        explicit CommandList(Device& device, QueueType queue = QueueType::Graphics);
        ~CommandList();

        CommandList(const CommandList&) = delete;
        auto operator=(const CommandList&) -> CommandList& = delete;
        CommandList(CommandList&& other) noexcept;
        auto operator=(CommandList&& other) noexcept -> CommandList&;

        // Binds the pipeline and the bindless table (set 1)
        auto Bind(const Pipeline& pipeline) -> void;

        // Push descriptors (set 0) for the bound pipeline; the descriptor type comes from the shader
        auto Bind(uint32_t binding, const GpuBuffer& buffer) -> void;
        auto Bind(uint32_t binding, const Image& image) -> void;
        auto Bind(uint32_t binding, const Image& image, const Sampler& sampler) -> void;
        auto Bind(uint32_t binding, const Sampler& sampler) -> void;

        auto PushConstants(const void* data, uint32_t size) -> void;

        template<typename T>
        auto PushConstants(const T& data) -> void
        {
            PushConstants(&data, sizeof(T));
        }

        // Compute
        auto Dispatch(uint32_t groupsX, uint32_t groupsY = 1, uint32_t groupsZ = 1) -> void;

        // Graphics. BeginRendering also sets the viewport and scissor to the attachment size.
        auto BeginRendering(const RenderingSpecification& specification) -> void;
        auto EndRendering() -> void;
        auto SetViewport(float x, float y, float width, float height) -> void;
        auto SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) -> void;
        auto BindVertexBuffer(const GpuBuffer& buffer, uint64_t offset = 0) -> void;
        auto BindIndexBuffer(const GpuBuffer& buffer, IndexType type = IndexType::UInt32, uint64_t offset = 0) -> void;
        auto Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) -> void;
        auto DrawIndexed(
            uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0
        ) -> void;

        // Transfers
        auto CopyBuffer(
            const GpuBuffer& source, const GpuBuffer& destination, uint64_t size, uint64_t sourceOffset = 0, uint64_t destinationOffset = 0
        ) -> void;
        // The image must be in TRANSFER_DST (Access::TransferWrite)
        auto CopyBufferToImage(const GpuBuffer& source, const Image& destination) -> void;
        // The image must be in TRANSFER_SRC (Access::TransferRead)
        auto CopyImageToBuffer(const Image& source, const GpuBuffer& destination) -> void;

        // Barriers: everything before (used as before) finishes before anything after (used as after) starts
        auto Barrier(Access before, Access after) -> void;
        auto Barrier(const GpuBuffer& buffer, Access before, Access after) -> void;
        auto Barrier(const Image& image, Access before, Access after) -> void;

        auto End() -> void;

        [[nodiscard]] auto GetHandle() const -> VkCommandBuffer { return m_CommandBuffer; }
        [[nodiscard]] auto GetDevice() const -> Device& { return *m_Device; }
        [[nodiscard]] auto GetQueueType() const -> QueueType { return m_QueueType; }
        [[nodiscard]] auto IsRecording() const -> bool { return m_Recording; }
        [[nodiscard]] auto IsSubmitted() const -> bool { return m_Submitted; }

    private:
        friend class Device;
        auto MarkSubmitted() -> void { m_Submitted = true; }

        auto PushDescriptor(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo) -> void;
        auto Release() -> void;

    private:
        Device* m_Device = nullptr;
        QueueType m_QueueType = QueueType::Graphics;
        VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
        const Pipeline* m_BoundPipeline = nullptr;
        bool m_Recording = false;
        bool m_Submitted = false;
    };
}

// Tracy GPU zone around the rest of the scope; a no-op on queues that can't write timestamps
#define EP_GPU_ZONE(commandList, name)                                                                                                     \
    TracyVkNamedZone(                                                                                                                      \
        (commandList).GetDevice().GetQueue((commandList).GetQueueType()).GetTracyContext(), TracyConcat(epGpuZone, TracyLine),             \
        (commandList).GetHandle(), name, (commandList).GetDevice().GetQueue((commandList).GetQueueType()).GetTracyContext() != nullptr     \
    )
