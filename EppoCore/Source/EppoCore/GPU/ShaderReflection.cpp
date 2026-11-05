#include "pch.h"
#include "ShaderReflection.h"

#include "EppoCore/GPU/BindlessTable.h"

#include <spirv_cross/spirv_cross.hpp>

namespace Eppo
{
    auto ShaderReflection::Reflect(const std::span<const uint32_t> spirv, const ShaderStage stage, const std::string_view name)
        -> std::optional<ShaderReflection>
    {
        ZoneScoped;

        try
        {
            spirv_cross::Compiler compiler(spirv.data(), spirv.size());
            const spirv_cross::ShaderResources resources = compiler.get_shader_resources();

            ShaderReflection reflection;
            bool valid = true;

            const auto add = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& list, const auto& getType)
            {
                for (const spirv_cross::Resource& resource : list)
                {
                    const uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                    const uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

                    if (set == BindlessTable::Set)
                    {
                        reflection.UsesBindlessTable = true;
                        continue;
                    }

                    if (set != 0)
                    {
                        Log::Error(
                            "Shader {}: {} uses descriptor set {}; only set 0 (push descriptors) and set 1 (bindless table) exist", name,
                            resource.name, set
                        );
                        valid = false;
                        continue;
                    }

                    const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);

                    uint32_t count = 1;
                    for (size_t i = 0; i < type.array.size(); i++)
                    {
                        if (type.array[i] == 0 || !type.array_size_literal[i])
                        {
                            Log::Error(
                                "Shader {}: {} is a runtime-sized array, which only the bindless table supports", name, resource.name
                            );
                            valid = false;
                        }

                        count *= std::max(type.array[i], 1u);
                    }

                    reflection.Bindings.push_back(
                        {
                            .Binding = binding,
                            .Type = getType(type),
                            .Count = count,
                            .Stages = static_cast<VkShaderStageFlags>(ToVkShaderStage(stage)),
                        }
                    );
                }
            };

            const auto fixed = [](const VkDescriptorType descriptorType)
            {
                return [descriptorType](const spirv_cross::SPIRType&)
                {
                    return descriptorType;
                };
            };

            add(resources.uniform_buffers, fixed(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
            add(resources.storage_buffers, fixed(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
            add(resources.sampled_images, fixed(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
            add(resources.separate_samplers, fixed(VK_DESCRIPTOR_TYPE_SAMPLER));
            add(resources.separate_images,
                [](const spirv_cross::SPIRType& type)
                {
                    return type.image.dim == spv::DimBuffer ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                });
            add(resources.storage_images,
                [](const spirv_cross::SPIRType& type)
                {
                    return type.image.dim == spv::DimBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                });

            if (!resources.push_constant_buffers.empty())
            {
                const auto& pushConstants = resources.push_constant_buffers.front();
                reflection.PushConstantSize =
                    static_cast<uint32_t>(compiler.get_declared_struct_size(compiler.get_type(pushConstants.base_type_id)));
            }

            if (stage == ShaderStage::Compute)
            {
                for (uint32_t i = 0; i < 3; i++)
                    reflection.LocalSize[i] = compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, i);
            }

            if (!valid)
                return std::nullopt;

            return reflection;
        }
        catch (const std::exception& exception)
        {
            Log::Error("Reflecting shader {} failed: {}", name, exception.what());
            return std::nullopt;
        }
    }
}
