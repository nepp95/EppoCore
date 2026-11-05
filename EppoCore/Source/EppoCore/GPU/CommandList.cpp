#include "pch.h"
#include "CommandList.h"

#include "EppoCore/GPU/BindlessTable.h"
#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/GpuBuffer.h"
#include "EppoCore/GPU/Image.h"
#include "EppoCore/GPU/Pipeline.h"
#include "EppoCore/GPU/Sampler.h"

namespace Eppo
{
    namespace
    {
        struct AccessInfo
        {
            VkPipelineStageFlags2 Stages = VK_PIPELINE_STAGE_2_NONE;
            VkAccessFlags2 Access = VK_ACCESS_2_NONE;
            VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
        };

        constexpr VkPipelineStageFlags2 s_AllShaderStages =
            VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

        auto GetAccessInfo(const Access access) -> AccessInfo
        {
            switch (access)
            {
                case Access::None:
                    return {};
                case Access::HostRead:
                    return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };
                case Access::HostWrite:
                    return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };
                case Access::TransferRead:
                    return { VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL };
                case Access::TransferWrite:
                    return { VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL };
                case Access::IndirectRead:
                    return { VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };
                case Access::VertexBuffer:
                    return { VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
                             VK_IMAGE_LAYOUT_GENERAL };
                case Access::IndexBuffer:
                    return { VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };
                case Access::UniformRead:
                    return { s_AllShaderStages, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };
                case Access::ComputeRead:
                    return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };
                case Access::ComputeWrite:
                    return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };
                case Access::FragmentRead:
                    return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
                case Access::ShaderRead:
                    return { s_AllShaderStages, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
                case Access::ColorAttachment:
                    return {
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    };
                case Access::DepthAttachment:
                    return {
                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    };
                case Access::Present:
                    return { VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
            }

            return {};
        }

        auto ToVkLoadOp(const LoadOp load) -> VkAttachmentLoadOp
        {
            switch (load)
            {
                case LoadOp::Load:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                case LoadOp::Clear:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                case LoadOp::DontCare:
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }

            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }

        auto ToImageLayout(const VkDescriptorType type) -> VkImageLayout
        {
            return type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }

    CommandList::CommandList(Device& device, const QueueType queue)
        : m_Device(&device), m_QueueType(queue)
    {
        ZoneScoped;

        const VkCommandBufferAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_Device->GetQueue(m_QueueType).GetCommandPool(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        EP_VK_CHECK(vkAllocateCommandBuffers(m_Device->GetHandle(), &allocateInfo, &m_CommandBuffer));

        constexpr VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        EP_VK_CHECK(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
        m_Recording = true;
    }

    CommandList::~CommandList()
    {
        Release();
    }

    CommandList::CommandList(CommandList&& other) noexcept
        : m_Device(other.m_Device),
          m_QueueType(other.m_QueueType),
          m_CommandBuffer(std::exchange(other.m_CommandBuffer, VK_NULL_HANDLE)),
          m_BoundPipeline(std::exchange(other.m_BoundPipeline, nullptr)),
          m_Recording(std::exchange(other.m_Recording, false)),
          m_Submitted(std::exchange(other.m_Submitted, false))
    {
    }

    auto CommandList::operator=(CommandList&& other) noexcept -> CommandList&
    {
        if (this != &other)
        {
            Release();
            m_Device = other.m_Device;
            m_QueueType = other.m_QueueType;
            m_CommandBuffer = std::exchange(other.m_CommandBuffer, VK_NULL_HANDLE);
            m_BoundPipeline = std::exchange(other.m_BoundPipeline, nullptr);
            m_Recording = std::exchange(other.m_Recording, false);
            m_Submitted = std::exchange(other.m_Submitted, false);
        }

        return *this;
    }

    auto CommandList::Bind(const Pipeline& pipeline) -> void
    {
        EP_ASSERT(m_Recording);
        EP_ASSERT(pipeline.IsValid());

        m_BoundPipeline = &pipeline;
        vkCmdBindPipeline(m_CommandBuffer, pipeline.GetBindPoint(), pipeline.GetHandle());

        const VkDescriptorSet bindlessSet = m_Device->GetBindlessTable().GetSet();
        vkCmdBindDescriptorSets(
            m_CommandBuffer, pipeline.GetBindPoint(), pipeline.GetLayout().GetHandle(), BindlessTable::Set, 1, &bindlessSet, 0, nullptr
        );
    }

    auto CommandList::Bind(const uint32_t binding, const GpuBuffer& buffer) -> void
    {
        const VkDescriptorBufferInfo bufferInfo{
            .buffer = buffer.GetHandle(),
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };

        PushDescriptor(binding, &bufferInfo, nullptr);
    }

    auto CommandList::Bind(const uint32_t binding, const Image& image) -> void
    {
        const ShaderBinding* shaderBinding = m_BoundPipeline ? m_BoundPipeline->GetLayout().FindBinding(binding) : nullptr;

        const VkDescriptorImageInfo imageInfo{
            .imageView = image.GetView(),
            .imageLayout = shaderBinding ? ToImageLayout(shaderBinding->Type) : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        PushDescriptor(binding, nullptr, &imageInfo);
    }

    auto CommandList::Bind(const uint32_t binding, const Image& image, const Sampler& sampler) -> void
    {
        const VkDescriptorImageInfo imageInfo{
            .sampler = sampler.GetHandle(),
            .imageView = image.GetView(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        PushDescriptor(binding, nullptr, &imageInfo);
    }

    auto CommandList::Bind(const uint32_t binding, const Sampler& sampler) -> void
    {
        const VkDescriptorImageInfo samplerInfo{
            .sampler = sampler.GetHandle(),
        };

        PushDescriptor(binding, nullptr, &samplerInfo);
    }

    auto CommandList::PushConstants(const void* data, const uint32_t size) -> void
    {
        EP_ASSERT(m_Recording);
        EP_ASSERT(m_BoundPipeline);

        const PipelineLayout& layout = m_BoundPipeline->GetLayout();
        if (size > layout.GetPushConstantSize())
        {
            Log::Error(
                "Pipeline {} takes {} bytes of push constants, got {}", m_BoundPipeline->GetName(), layout.GetPushConstantSize(), size
            );
            return;
        }

        vkCmdPushConstants(m_CommandBuffer, layout.GetHandle(), layout.GetPushConstantStages(), 0, size, data);
    }

    auto CommandList::Dispatch(const uint32_t groupsX, const uint32_t groupsY, const uint32_t groupsZ) -> void
    {
        EP_ASSERT(m_Recording);
        vkCmdDispatch(m_CommandBuffer, groupsX, groupsY, groupsZ);
    }

    auto CommandList::BeginRendering(const RenderingSpecification& specification) -> void
    {
        EP_ASSERT(m_Recording);

        std::vector<VkRenderingAttachmentInfo> colorAttachments;
        uint32_t width = 0;
        uint32_t height = 0;

        for (const ColorAttachment& attachment : specification.ColorAttachments)
        {
            EP_ASSERT(attachment.Target);
            width = attachment.Target->GetWidth();
            height = attachment.Target->GetHeight();

            VkRenderingAttachmentInfo info{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = attachment.Target->GetView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = ToVkLoadOp(attachment.Load),
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            };
            const glm::vec4& clear = attachment.ClearColor;
            info.clearValue.color = {
                { clear.r, clear.g, clear.b, clear.a }
            };

            colorAttachments.push_back(info);
        }

        VkRenderingAttachmentInfo depthAttachment{};
        const Image* depthTarget = specification.Depth.Target;
        if (depthTarget)
        {
            width = depthTarget->GetWidth();
            height = depthTarget->GetHeight();

            depthAttachment = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = depthTarget->GetView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = ToVkLoadOp(specification.Depth.Load),
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            };
            depthAttachment.clearValue.depthStencil = { specification.Depth.ClearDepth, 0 };
        }

        const VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = { { 0, 0 }, { width, height } },
            .layerCount = 1,
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
            .pColorAttachments = colorAttachments.data(),
            .pDepthAttachment = depthTarget ? &depthAttachment : nullptr,
        };

        vkCmdBeginRendering(m_CommandBuffer, &renderingInfo);
        SetViewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
        SetScissor(0, 0, width, height);
    }

    auto CommandList::EndRendering() -> void
    {
        EP_ASSERT(m_Recording);
        vkCmdEndRendering(m_CommandBuffer);
    }

    auto CommandList::SetViewport(const float x, const float y, const float width, const float height) -> void
    {
        const VkViewport viewport{ x, y, width, height, 0.0f, 1.0f };
        vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
    }

    auto CommandList::SetScissor(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) -> void
    {
        const VkRect2D scissor{
            { x,     y      },
            { width, height }
        };
        vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
    }

    auto CommandList::BindVertexBuffer(const GpuBuffer& buffer, const uint64_t offset) -> void
    {
        const VkBuffer handle = buffer.GetHandle();
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &handle, &offset);
    }

    auto CommandList::BindIndexBuffer(const GpuBuffer& buffer, const IndexType type, const uint64_t offset) -> void
    {
        vkCmdBindIndexBuffer(
            m_CommandBuffer, buffer.GetHandle(), offset, type == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32
        );
    }

    auto
    CommandList::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
        -> void
    {
        vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    auto CommandList::DrawIndexed(
        const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset,
        const uint32_t firstInstance
    ) -> void
    {
        vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    auto CommandList::CopyBuffer(
        const GpuBuffer& source, const GpuBuffer& destination, const uint64_t size, const uint64_t sourceOffset,
        const uint64_t destinationOffset
    ) -> void
    {
        EP_ASSERT(m_Recording);

        const VkBufferCopy region{
            .srcOffset = sourceOffset,
            .dstOffset = destinationOffset,
            .size = size,
        };

        vkCmdCopyBuffer(m_CommandBuffer, source.GetHandle(), destination.GetHandle(), 1, &region);
    }

    auto CommandList::CopyBufferToImage(const GpuBuffer& source, const Image& destination) -> void
    {
        EP_ASSERT(m_Recording);

        const VkBufferImageCopy region{
            .imageSubresource = { destination.GetAspect(), 0, 0, 1 },
            .imageExtent = { destination.GetWidth(), destination.GetHeight(), 1 },
        };

        vkCmdCopyBufferToImage(
            m_CommandBuffer, source.GetHandle(), destination.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region
        );
    }

    auto CommandList::CopyImageToBuffer(const Image& source, const GpuBuffer& destination) -> void
    {
        EP_ASSERT(m_Recording);

        const VkBufferImageCopy region{
            .imageSubresource = { source.GetAspect(), 0, 0, 1 },
            .imageExtent = { source.GetWidth(), source.GetHeight(), 1 },
        };

        vkCmdCopyImageToBuffer(
            m_CommandBuffer, source.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination.GetHandle(), 1, &region
        );
    }

    auto CommandList::Barrier(const Access before, const Access after) -> void
    {
        EP_ASSERT(m_Recording);

        const AccessInfo source = GetAccessInfo(before);
        const AccessInfo destination = GetAccessInfo(after);

        const VkMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = source.Stages,
            .srcAccessMask = source.Access,
            .dstStageMask = destination.Stages,
            .dstAccessMask = destination.Access,
        };

        const VkDependencyInfo dependency{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .memoryBarrierCount = 1,
            .pMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(m_CommandBuffer, &dependency);
    }

    auto CommandList::Barrier(const GpuBuffer& buffer, const Access before, const Access after) -> void
    {
        EP_ASSERT(m_Recording);

        const AccessInfo source = GetAccessInfo(before);
        const AccessInfo destination = GetAccessInfo(after);

        const VkBufferMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask = source.Stages,
            .srcAccessMask = source.Access,
            .dstStageMask = destination.Stages,
            .dstAccessMask = destination.Access,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = buffer.GetHandle(),
            .offset = 0,
            .size = VK_WHOLE_SIZE,
        };

        const VkDependencyInfo dependency{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(m_CommandBuffer, &dependency);
    }

    auto CommandList::Barrier(const Image& image, const Access before, const Access after) -> void
    {
        EP_ASSERT(m_Recording);

        const AccessInfo source = GetAccessInfo(before);
        const AccessInfo destination = GetAccessInfo(after);

        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = source.Stages,
            .srcAccessMask = source.Access,
            .dstStageMask = destination.Stages,
            .dstAccessMask = destination.Access,
            .oldLayout = source.Layout,
            .newLayout = destination.Layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image.GetHandle(),
            .subresourceRange = { image.GetAspect(), 0, 1, 0, 1 },
        };

        const VkDependencyInfo dependency{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(m_CommandBuffer, &dependency);
    }

    auto CommandList::End() -> void
    {
        EP_ASSERT(m_Recording);

        EP_VK_CHECK(vkEndCommandBuffer(m_CommandBuffer));
        m_Recording = false;
    }

    auto
    CommandList::PushDescriptor(const uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo)
        -> void
    {
        EP_ASSERT(m_Recording);

        if (!m_BoundPipeline)
        {
            Log::Error("Binding {} without a bound pipeline", binding);
            return;
        }

        const PipelineLayout& layout = m_BoundPipeline->GetLayout();
        const ShaderBinding* shaderBinding = layout.FindBinding(binding);
        if (!shaderBinding)
        {
            Log::Error("Pipeline {} has no binding {}", m_BoundPipeline->GetName(), binding);
            return;
        }

        const VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = shaderBinding->Type,
            .pImageInfo = imageInfo,
            .pBufferInfo = bufferInfo,
        };

        m_Device->GetPushDescriptorFunction()(m_CommandBuffer, m_BoundPipeline->GetBindPoint(), layout.GetHandle(), 0, 1, &write);
    }

    auto CommandList::Release() -> void
    {
        if (!m_CommandBuffer)
            return;

        // Freed once all work submitted so far is done, which includes this command buffer when it was submitted
        m_Device->Defer(
            [device = m_Device->GetHandle(), pool = m_Device->GetQueue(m_QueueType).GetCommandPool(), commandBuffer = m_CommandBuffer]
            {
                vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
            }
        );

        m_CommandBuffer = VK_NULL_HANDLE;
    }
}
